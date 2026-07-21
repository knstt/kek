#ifndef KEK_RUNTIME_APP_H
#define KEK_RUNTIME_APP_H

#include <stddef.h>

#include "hook.h"
#include "runtime.h"
#include "state_store.h"

typedef struct KekRuntimeApp {
    KekRuntime runtime;
    KekStateStore state_store;
    KekHookRegistry hook_registry;
} KekRuntimeApp;

int kek_runtime_app_init(KekRuntimeApp* app, void* app_context);
int kek_runtime_app_bind_hooks(KekRuntimeApp* app,
                               const KekHookDescriptor* descriptors,
                               size_t descriptor_count);
void kek_runtime_app_destroy(KekRuntimeApp* app);
KekRuntime* kek_runtime_app_runtime(KekRuntimeApp* app);
const KekRuntime* kek_runtime_app_runtime_const(const KekRuntimeApp* app);
KekStateStore* kek_runtime_app_store(KekRuntimeApp* app);
const KekStateStore* kek_runtime_app_store_const(const KekRuntimeApp* app);
int kek_runtime_app_dispatch(KekRuntimeApp* app);

#endif
