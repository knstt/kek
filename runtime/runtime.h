#ifndef KEK_RUNTIME_RUNTIME_H
#define KEK_RUNTIME_RUNTIME_H

#include <stddef.h>
#include <termios.h>

#include "event.h"
#include "state.h"

#define KEK_RUNTIME_MAX_STATES 64

typedef struct KekRuntime {
    KekEventDispatcher events;
    KekRuntimeState states[KEK_RUNTIME_MAX_STATES];
    size_t state_count;
    int quit_requested;
    int raw_mode_enabled;
    struct termios original_termios;
} KekRuntime;

void kek_runtime_init(KekRuntime* runtime);
void kek_runtime_destroy(KekRuntime* runtime);
KekEventDispatcher* kek_runtime_events(KekRuntime* runtime);
int kek_runtime_register_state(KekRuntime* runtime, const KekRuntimeState* state);
KekRuntimeState* kek_runtime_get_state(KekRuntime* runtime, size_t state_id);
void kek_runtime_request_quit(KekRuntime* runtime);
int kek_runtime_publish_state_changed(KekRuntime* runtime, void* source);
int kek_runtime_run(KekRuntime* runtime);
int kek_runtime_drain(KekRuntime* runtime);
int kek_runtime_enable_raw_mode(KekRuntime* runtime, int fd);
void kek_runtime_disable_raw_mode(KekRuntime* runtime, int fd);

#endif
