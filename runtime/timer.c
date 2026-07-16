#define _POSIX_C_SOURCE 200809L

#include "timer.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct KekTimerStateUpdate {
    uint64_t tick;
    uint32_t interval_ms;
    int enabled;
} KekTimerStateUpdate;

typedef struct KekStandardTimerState {
    uint64_t tick;
    uint32_t interval_ms;
    bool enabled;
} KekStandardTimerState;

static struct timeval timer_now(void) {
    struct timespec now;
    if (clock_gettime(CLOCK_MONOTONIC, &now) == 0) {
        struct timeval result;
        result.tv_sec = now.tv_sec;
        result.tv_usec = now.tv_nsec / 1000;
        return result;
    }
    struct timeval fallback = {0, 0};
    return fallback;
}

static struct timeval timer_add_ms(struct timeval value, uint32_t interval_ms) {
    value.tv_sec += (time_t)(interval_ms / 1000u);
    value.tv_usec += (suseconds_t)(interval_ms % 1000u) * 1000;
    if (value.tv_usec >= 1000000) {
        value.tv_sec++;
        value.tv_usec -= 1000000;
    }
    return value;
}

static int timer_cmp(struct timeval lhs, struct timeval rhs) {
    if (lhs.tv_sec != rhs.tv_sec) {
        return lhs.tv_sec < rhs.tv_sec ? -1 : 1;
    }
    if (lhs.tv_usec != rhs.tv_usec) {
        return lhs.tv_usec < rhs.tv_usec ? -1 : 1;
    }
    return 0;
}

static struct timeval timer_remaining(struct timeval now, struct timeval deadline) {
    struct timeval result = {0, 0};
    if (timer_cmp(deadline, now) <= 0) {
        return result;
    }
    result.tv_sec = deadline.tv_sec - now.tv_sec;
    result.tv_usec = deadline.tv_usec - now.tv_usec;
    if (result.tv_usec < 0) {
        result.tv_sec--;
        result.tv_usec += 1000000;
    }
    return result;
}

static void update_timer_state(void* draft, void* context) {
    KekStandardTimerState* state = (KekStandardTimerState*)draft;
    KekTimerStateUpdate* update = (KekTimerStateUpdate*)context;
    state->tick = update->tick;
    state->interval_ms = update->interval_ms;
    state->enabled = update->enabled != 0;
}

static void timer_publish(KekTimer* timer) {
    if (!timer || !timer->store) {
        return;
    }
    KekTimerStateUpdate update = {timer->tick, timer->interval_ms, timer->enabled};
    kek_state_store_update(timer->store, timer->slot_id, update_timer_state, &update);
}

static int timer_prepare(KekRuntime* runtime, KekRuntimeState* state,
                         fd_set* read_fds, fd_set* write_fds, int* max_fd,
                         struct timeval* timeout) {
    (void)runtime;
    (void)read_fds;
    (void)write_fds;
    (void)max_fd;
    KekTimer* timer = (KekTimer*)state->data;
    if (!timer || !timer->enabled || timer->interval_ms == 0 || !timeout) {
        return 0;
    }
    *timeout = timer_remaining(timer_now(), timer->next_tick);
    return 0;
}

static void timer_ready(KekRuntime* runtime, KekRuntimeState* state,
                        const fd_set* read_fds, const fd_set* write_fds) {
    (void)runtime;
    (void)read_fds;
    (void)write_fds;
    KekTimer* timer = (KekTimer*)state->data;
    if (!timer || !timer->enabled || timer->interval_ms == 0) {
        return;
    }

    struct timeval now = timer_now();
    if (timer_cmp(now, timer->next_tick) < 0) {
        return;
    }

    do {
        timer->tick++;
        timer->next_tick = timer_add_ms(timer->next_tick, timer->interval_ms);
    } while (timer_cmp(now, timer->next_tick) >= 0);
    timer_publish(timer);
}

static void timer_destroy(KekRuntimeState* state) {
    free(state->data);
    state->data = NULL;
}

int kek_runtime_register_timer(KekRuntime* runtime, KekStateStore* store,
                               size_t slot_id, uint32_t interval_ms) {
    if (!runtime || !store || interval_ms == 0) {
        return -1;
    }

    KekTimer* timer = (KekTimer*)calloc(1, sizeof(*timer));
    if (!timer) {
        return -1;
    }
    timer->store = store;
    timer->slot_id = slot_id;
    timer->interval_ms = interval_ms;
    timer->enabled = 1;
    timer->next_tick = timer_add_ms(timer_now(), interval_ms);

    KekRuntimeState state;
    memset(&state, 0, sizeof(state));
    state.kind = KEK_RUNTIME_STATE_TIMER;
    state.data = timer;
    state.prepare = timer_prepare;
    state.ready = timer_ready;
    state.destroy = timer_destroy;

    int id = kek_runtime_register_state(runtime, &state);
    if (id < 0) {
        free(timer);
    } else {
        timer_publish(timer);
    }
    return id;
}

KekTimer* kek_runtime_get_timer(KekRuntime* runtime, size_t state_id) {
    KekRuntimeState* state = kek_runtime_get_state(runtime, state_id);
    if (!state || state->kind != KEK_RUNTIME_STATE_TIMER) {
        return NULL;
    }
    return (KekTimer*)state->data;
}

int kek_timer_set_enabled(KekTimer* timer, int enabled) {
    if (!timer) {
        return 0;
    }
    timer->enabled = enabled != 0;
    if (timer->enabled) {
        timer->next_tick = timer_add_ms(timer_now(), timer->interval_ms);
    }
    timer_publish(timer);
    return 1;
}
