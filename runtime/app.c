#include "app.h"

#include <string.h>

int kek_runtime_app_init(KekRuntimeApp* app, void* app_context) {
    if (!app) {
        return 0;
    }

    memset(app, 0, sizeof(*app));
    kek_runtime_init(&app->runtime);
    kek_state_store_init(&app->state_store, &app->runtime);
    kek_hook_registry_init(&app->hook_registry, &app->runtime, &app->state_store,
                           app_context);
    return 1;
}

int kek_runtime_app_bind_hooks(KekRuntimeApp* app,
                               const KekHookDescriptor* descriptors,
                               size_t descriptor_count) {
    if (!app) {
        return 0;
    }
    if (!kek_hook_registry_add_many(&app->hook_registry, descriptors,
                                    descriptor_count)) {
        return 0;
    }
    kek_hook_registry_attach(&app->hook_registry);
    return app->hook_registry.attached;
}

void kek_runtime_app_destroy(KekRuntimeApp* app) {
    if (!app) {
        return;
    }

    kek_hook_registry_detach(&app->hook_registry);
    kek_state_store_destroy(&app->state_store);
    kek_runtime_destroy(&app->runtime);
    memset(app, 0, sizeof(*app));
}

KekRuntime* kek_runtime_app_runtime(KekRuntimeApp* app) {
    return app ? &app->runtime : 0;
}

const KekRuntime* kek_runtime_app_runtime_const(const KekRuntimeApp* app) {
    return app ? &app->runtime : 0;
}

KekStateStore* kek_runtime_app_store(KekRuntimeApp* app) {
    return app ? &app->state_store : 0;
}

const KekStateStore* kek_runtime_app_store_const(const KekRuntimeApp* app) {
    return app ? &app->state_store : 0;
}

int kek_runtime_app_dispatch(KekRuntimeApp* app) {
    if (!app) {
        return 0;
    }
    return kek_event_dispatch_pending(kek_runtime_events(&app->runtime));
}
