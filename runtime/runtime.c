#include "runtime.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

void kek_runtime_init(KekRuntime* runtime) {
    memset(runtime, 0, sizeof(*runtime));
    kek_event_dispatcher_init(&runtime->events);
}

void kek_runtime_destroy(KekRuntime* runtime) {
    if (!runtime) {
        return;
    }

    for (size_t i = runtime->state_count; i > 0; i--) {
        KekRuntimeState* state = &runtime->states[i - 1];
        if (state->destroy) {
            state->destroy(state);
        }
    }
    runtime->state_count = 0;
}

KekEventDispatcher* kek_runtime_events(KekRuntime* runtime) {
    return runtime ? &runtime->events : NULL;
}

int kek_runtime_register_state(KekRuntime* runtime, const KekRuntimeState* state) {
    if (!runtime || !state || runtime->state_count >= KEK_RUNTIME_MAX_STATES) {
        return -1;
    }

    runtime->states[runtime->state_count] = *state;
    return (int)runtime->state_count++;
}

KekRuntimeState* kek_runtime_get_state(KekRuntime* runtime, size_t state_id) {
    if (!runtime || state_id >= runtime->state_count) {
        return NULL;
    }
    return &runtime->states[state_id];
}

void kek_runtime_request_quit(KekRuntime* runtime) {
    if (runtime) {
        runtime->quit_requested = 1;
    }
}

int kek_runtime_publish_state_changed(KekRuntime* runtime, void* source) {
    if (!runtime) {
        return 0;
    }

    KekEvent event;
    memset(&event, 0, sizeof(event));
    event.type = KEK_EVENT_STATE_CHANGED;
    event.source = source;
    return kek_event_publish(&runtime->events, &event);
}

int kek_runtime_enable_raw_mode(KekRuntime* runtime, int fd) {
    if (!runtime || !isatty(fd)) {
        return 0;
    }

    if (tcgetattr(fd, &runtime->original_termios) == -1) {
        perror("tcgetattr");
        return -1;
    }

    struct termios raw = runtime->original_termios;
    raw.c_lflag &= ~(ICANON | ECHO);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 0;

    if (tcsetattr(fd, TCSAFLUSH, &raw) == -1) {
        perror("tcsetattr");
        return -1;
    }

    runtime->raw_mode_enabled = 1;
    return 0;
}

void kek_runtime_disable_raw_mode(KekRuntime* runtime, int fd) {
    if (runtime && runtime->raw_mode_enabled) {
        tcsetattr(fd, TCSAFLUSH, &runtime->original_termios);
        runtime->raw_mode_enabled = 0;
    }
}

int kek_runtime_run(KekRuntime* runtime) {
    if (!runtime) {
        return -1;
    }

    while (!runtime->quit_requested) {
        fd_set read_fds;
        fd_set write_fds;
        FD_ZERO(&read_fds);
        FD_ZERO(&write_fds);
        int max_fd = -1;

        for (size_t i = 0; i < runtime->state_count; i++) {
            KekRuntimeState* state = &runtime->states[i];
            if (state->prepare) {
                state->prepare(runtime, state, &read_fds, &write_fds, &max_fd);
            }
        }

        if (kek_event_has_pending(&runtime->events)) {
            kek_event_dispatch_pending(&runtime->events);
            continue;
        }

        if (max_fd < 0) {
            break;
        }

        int result = select(max_fd + 1, &read_fds, &write_fds, NULL, NULL);
        if (result < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("select");
            return -1;
        }

        for (size_t i = 0; i < runtime->state_count; i++) {
            KekRuntimeState* state = &runtime->states[i];
            if (state->ready) {
                state->ready(runtime, state, &read_fds, &write_fds);
            }
        }

        kek_event_dispatch_pending(&runtime->events);
    }

    kek_event_dispatch_pending(&runtime->events);
    return kek_runtime_drain(runtime);
}

int kek_runtime_drain(KekRuntime* runtime) {
    if (!runtime) {
        return -1;
    }

    for (;;) {
        int has_state_work = 0;
        for (size_t i = 0; i < runtime->state_count; i++) {
            KekRuntimeState* state = &runtime->states[i];
            if (state->has_work && state->has_work(state)) {
                has_state_work = 1;
                break;
            }
        }

        if (!has_state_work && !kek_event_has_pending(&runtime->events)) {
            break;
        }

        kek_event_dispatch_pending(&runtime->events);

        has_state_work = 0;
        for (size_t i = 0; i < runtime->state_count; i++) {
            KekRuntimeState* state = &runtime->states[i];
            if (state->has_work && state->has_work(state)) {
                has_state_work = 1;
                break;
            }
        }

        if (!has_state_work) {
            continue;
        }

        fd_set read_fds;
        fd_set write_fds;
        FD_ZERO(&read_fds);
        FD_ZERO(&write_fds);
        int max_fd = -1;

        for (size_t i = 0; i < runtime->state_count; i++) {
            KekRuntimeState* state = &runtime->states[i];
            if (state->prepare) {
                state->prepare(runtime, state, &read_fds, &write_fds, &max_fd);
            }
        }

        if (max_fd < 0) {
            continue;
        }

        int result = select(max_fd + 1, NULL, &write_fds, NULL, NULL);
        if (result < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("select");
            return -1;
        }

        for (size_t i = 0; i < runtime->state_count; i++) {
            KekRuntimeState* state = &runtime->states[i];
            if (state->ready) {
                state->ready(runtime, state, &read_fds, &write_fds);
            }
        }
    }

    return 0;
}
