#include <stddef.h>
#include <stdio.h>

#include "dynamic_hook_smoke.h"
#include "runtime/runtime.h"
#include "runtime/state_storage.h"

#define DYNAMIC_SMOKE_STATE_TYPE 100

typedef struct DynamicSmokeState {
    int value;
} DynamicSmokeState;

static void dynamic_smoke_default(void* state) {
    DynamicSmokeState* smoke = (DynamicSmokeState*)state;
    smoke->value = 0;
}

static int dynamic_smoke_check(const void* state) {
    return state != NULL;
}

static void increment_state(void* draft, void* context) {
    (void)context;
    DynamicSmokeState* smoke = (DynamicSmokeState*)draft;
    smoke->value++;
}

static int dispatch_one(KekStateStore* store, KekRuntime* runtime, size_t slot_id) {
    return kek_state_store_update_fields(store, slot_id, increment_state, NULL, 1) &&
           kek_event_dispatch_pending(kek_runtime_events(runtime));
}

int main(int argc, char** argv) {
    if (argc != 4) {
        fprintf(stderr, "usage: %s <hook-v1> <missing-hook-lib> <hook-v2>\n", argv[0]);
        return 2;
    }

    KekRuntime runtime;
    KekStateStore store;
    KekHookRegistry registry;
    DynamicHookSmokeApp app = {0};

    kek_runtime_init(&runtime);
    kek_state_store_init(&store, &runtime);
    kek_hook_registry_init(&registry, &runtime, &store, &app);

    KekStateDescriptor state_descriptor = {
        DYNAMIC_SMOKE_STATE_TYPE,
        "DynamicSmokeState",
        sizeof(DynamicSmokeState),
        dynamic_smoke_default,
        dynamic_smoke_check,
        NULL,
    };
    size_t slot_id = kek_state_store_add_default(&store, &state_descriptor);
    if (slot_id == KEK_STATE_INVALID_ID) {
        return 1;
    }

    KekHookDescriptor hook_descriptor = {
        "DynamicSmokeHook",
        KEK_EVENT_STATE_CHANGED,
        DYNAMIC_SMOKE_STATE_TYPE,
        slot_id,
        1,
        NULL,
        0,
        NULL,
        0,
        NULL,
    };
    if (!kek_hook_registry_add(&registry, &hook_descriptor)) {
        return 1;
    }
    kek_hook_registry_attach(&registry);

    if (!kek_hook_registry_load_library(&registry, argv[1]) ||
        !dispatch_one(&store, &runtime, slot_id) || app.hook_value != 1) {
        return 1;
    }
    if (kek_hook_registry_load_library(&registry, argv[2]) ||
        !dispatch_one(&store, &runtime, slot_id) || app.hook_value != 2) {
        return 1;
    }
    if (!kek_hook_registry_load_library(&registry, argv[3]) ||
        !dispatch_one(&store, &runtime, slot_id) || app.hook_value != 12) {
        return 1;
    }

    kek_hook_registry_detach(&registry);
    kek_hook_registry_unload_library(&registry);
    kek_state_store_destroy(&store);
    kek_runtime_destroy(&runtime);
    printf("runtime dynamic hook smoke passed\n");
    return 0;
}
