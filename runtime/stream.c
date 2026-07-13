#include "stream.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int stream_prepare(KekRuntime* runtime, KekRuntimeState* state,
                          fd_set* read_fds, fd_set* write_fds, int* max_fd) {
    (void)runtime;
    KekStream* stream = (KekStream*)state->data;
    if (!stream || stream->closed || stream->fd < 0) {
        return 0;
    }

    if (stream->mode == KEK_STREAM_READ) {
        FD_SET(stream->fd, read_fds);
    } else if (stream->length > 0) {
        FD_SET(stream->fd, write_fds);
    }

    if (stream->fd > *max_fd) {
        *max_fd = stream->fd;
    }
    return 0;
}

static void publish_stream_event(KekRuntime* runtime, KekStream* stream,
                                 KekEventType type, const char* data,
                                 size_t data_len, int error_code) {
    KekEvent event;
    memset(&event, 0, sizeof(event));
    event.type = type;
    event.source = stream;
    event.error_code = error_code;

    if (data && data_len > 0) {
        size_t to_copy = data_len < KEK_EVENT_DATA_CAPACITY ? data_len : KEK_EVENT_DATA_CAPACITY;
        memcpy(event.data, data, to_copy);
        event.data_len = to_copy;
    }

    kek_event_publish(kek_runtime_events(runtime), &event);
}

static void stream_ready(KekRuntime* runtime, KekRuntimeState* state,
                         const fd_set* read_fds, const fd_set* write_fds) {
    KekStream* stream = (KekStream*)state->data;
    if (!stream || stream->closed || stream->fd < 0) {
        return;
    }

    if (stream->mode == KEK_STREAM_READ && FD_ISSET(stream->fd, read_fds)) {
        char buffer[KEK_EVENT_DATA_CAPACITY];
        ssize_t bytes = read(stream->fd, buffer, sizeof(buffer));
        if (bytes > 0) {
            publish_stream_event(runtime, stream, KEK_EVENT_STREAM_DATA, buffer,
                                 (size_t)bytes, 0);
        } else if (bytes == 0) {
            stream->closed = 1;
            publish_stream_event(runtime, stream, KEK_EVENT_STREAM_EOF, NULL, 0, 0);
        } else if (errno != EAGAIN && errno != EWOULDBLOCK) {
            stream->closed = 1;
            publish_stream_event(runtime, stream, KEK_EVENT_STREAM_ERROR, NULL, 0, errno);
        }
    }

    if (stream->mode == KEK_STREAM_WRITE && stream->length > 0 &&
        FD_ISSET(stream->fd, write_fds)) {
        ssize_t written = write(stream->fd, stream->buffer, stream->length);
        if (written > 0) {
            size_t count = (size_t)written;
            memmove(stream->buffer, stream->buffer + count, stream->length - count);
            stream->length -= count;
        } else if (written < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
            stream->closed = 1;
            publish_stream_event(runtime, stream, KEK_EVENT_STREAM_ERROR, NULL, 0, errno);
        }
    }
}

static void stream_destroy(KekRuntimeState* state) {
    KekStream* stream = (KekStream*)state->data;
    if (!stream) {
        return;
    }

    if (stream->close_on_destroy && stream->fd >= 0) {
        close(stream->fd);
    }
    free(stream);
    state->data = NULL;
}

static int stream_has_work(const KekRuntimeState* state) {
    const KekStream* stream = (const KekStream*)state->data;
    return stream && stream->mode == KEK_STREAM_WRITE && stream->length > 0 &&
           !stream->closed;
}

int kek_runtime_register_stream(KekRuntime* runtime, int fd, KekStreamMode mode,
                                int close_on_destroy) {
    KekStream* stream = (KekStream*)calloc(1, sizeof(*stream));
    if (!stream) {
        return -1;
    }

    stream->fd = fd;
    stream->mode = mode;
    stream->close_on_destroy = close_on_destroy;

    KekRuntimeState state;
    memset(&state, 0, sizeof(state));
    state.kind = KEK_RUNTIME_STATE_STREAM;
    state.data = stream;
    state.prepare = stream_prepare;
    state.ready = stream_ready;
    state.has_work = stream_has_work;
    state.destroy = stream_destroy;

    int id = kek_runtime_register_state(runtime, &state);
    if (id < 0) {
        free(stream);
    }
    return id;
}

KekStream* kek_runtime_get_stream(KekRuntime* runtime, size_t state_id) {
    KekRuntimeState* state = kek_runtime_get_state(runtime, state_id);
    if (!state || state->kind != KEK_RUNTIME_STATE_STREAM) {
        return NULL;
    }
    return (KekStream*)state->data;
}

size_t kek_stream_write(KekStream* stream, const char* data) {
    return data ? kek_stream_write_raw(stream, data, strlen(data)) : 0;
}

size_t kek_stream_write_raw(KekStream* stream, const char* data, size_t len) {
    if (!stream || stream->mode != KEK_STREAM_WRITE || stream->closed || !data) {
        return 0;
    }

    size_t available = sizeof(stream->buffer) - stream->length;
    size_t to_copy = len < available ? len : available;
    if (to_copy > 0) {
        memcpy(stream->buffer + stream->length, data, to_copy);
        stream->length += to_copy;
    }
    return to_copy;
}

size_t kek_stream_read_buffer(KekStream* stream, char* output, size_t max_len) {
    if (!stream || stream->mode != KEK_STREAM_READ || !output || max_len == 0) {
        return 0;
    }

    size_t to_copy = stream->length < max_len ? stream->length : max_len;
    if (to_copy > 0) {
        memcpy(output, stream->buffer, to_copy);
        memmove(stream->buffer, stream->buffer + to_copy, stream->length - to_copy);
        stream->length -= to_copy;
    }
    return to_copy;
}
