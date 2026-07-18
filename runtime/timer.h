#ifndef KEK_RUNTIME_TIMER_H
#define KEK_RUNTIME_TIMER_H

#include <stddef.h>
#include <stdint.h>
#include <sys/time.h>

#include "runtime.h"
#include "state_storage.h"

typedef struct KekTimer {
    KekRuntime* runtime;
    KekStateStore* store;
    size_t slot_id;
    uint64_t tick;
    uint32_t interval_ms;
    int enabled;
    struct timeval next_tick;
} KekTimer;

int kek_runtime_register_timer(KekRuntime* runtime, KekStateStore* store,
                               size_t slot_id, uint32_t interval_ms);
KekTimer* kek_runtime_get_timer(KekRuntime* runtime, size_t state_id);
int kek_timer_set_enabled(KekTimer* timer, int enabled);

#endif
