#ifndef KEK_RUNTIME_STREAM_H
#define KEK_RUNTIME_STREAM_H

#include <stddef.h>

#include "runtime.h"

#define KEK_STREAM_BUFFER_CAPACITY 4096

typedef enum KekStreamMode {
    KEK_STREAM_READ = 0,
    KEK_STREAM_WRITE = 1
} KekStreamMode;

typedef struct KekStream {
    int fd;
    KekStreamMode mode;
    int close_on_destroy;
    int closed;
    char buffer[KEK_STREAM_BUFFER_CAPACITY];
    size_t length;
} KekStream;

int kek_runtime_register_stream(KekRuntime* runtime, int fd, KekStreamMode mode,
                                int close_on_destroy);
KekStream* kek_runtime_get_stream(KekRuntime* runtime, size_t state_id);
size_t kek_stream_write(KekStream* stream, const char* data);
size_t kek_stream_write_raw(KekStream* stream, const char* data, size_t len);

#endif
