#ifndef KEK_RUNTIME_STATE_H
#define KEK_RUNTIME_STATE_H

#include <sys/select.h>

typedef enum KekRuntimeStateKind {
    KEK_RUNTIME_STATE_STREAM = 0,
    KEK_RUNTIME_STATE_CUSTOM = 1
} KekRuntimeStateKind;

struct KekRuntime;
struct KekRuntimeState;

typedef int (*KekRuntimeStatePrepareFn)(struct KekRuntime* runtime,
                                        struct KekRuntimeState* state,
                                        fd_set* read_fds, fd_set* write_fds,
                                        int* max_fd);
typedef void (*KekRuntimeStateReadyFn)(struct KekRuntime* runtime,
                                       struct KekRuntimeState* state,
                                       const fd_set* read_fds,
                                       const fd_set* write_fds);
typedef int (*KekRuntimeStateHasWorkFn)(const struct KekRuntimeState* state);
typedef void (*KekRuntimeStateDestroyFn)(struct KekRuntimeState* state);

typedef struct KekRuntimeState {
    KekRuntimeStateKind kind;
    void* data;
    KekRuntimeStatePrepareFn prepare;
    KekRuntimeStateReadyFn ready;
    KekRuntimeStateHasWorkFn has_work;
    KekRuntimeStateDestroyFn destroy;
} KekRuntimeState;

#endif
