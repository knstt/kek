#include "hook.h"

#ifdef KEK_HOOK_DYNAMIC
#include <dlfcn.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#endif

#include "runtime.h"
#include "state_storage.h"
#include "trace.h"

static int hook_registry_event_handler(const KekEvent* event, void* context) {
    return kek_hook_registry_dispatch((KekHookRegistry*)context, event);
}

void kek_hook_registry_init(KekHookRegistry* registry, struct KekRuntime* runtime,
                            struct KekStateStore* state_store, void* app_context) {
    if (!registry) {
        return;
    }

    registry->runtime = runtime;
    registry->state_store = state_store;
    registry->app_context = app_context;
    registry->descriptor_count = 0;
    registry->attached = 0;
#ifdef KEK_HOOK_DYNAMIC
    registry->dynamic_library = NULL;
    registry->dynamic_library_path = NULL;
    registry->dynamic_loaded_path = NULL;
    registry->dynamic_generation = 0;
#endif
}

int kek_hook_registry_add(KekHookRegistry* registry,
                          const KekHookDescriptor* descriptor) {
    if (!registry || !descriptor ||
#ifndef KEK_HOOK_DYNAMIC
        !descriptor->run ||
#endif
        registry->descriptor_count >= KEK_HOOK_MAX_DESCRIPTORS) {
        return 0;
    }

    registry->descriptors[registry->descriptor_count++] = *descriptor;
    return 1;
}

int kek_hook_registry_add_many(KekHookRegistry* registry,
                               const KekHookDescriptor* descriptors,
                               size_t descriptor_count) {
    if (!registry || (!descriptors && descriptor_count > 0)) {
        return 0;
    }

    for (size_t i = 0; i < descriptor_count; i++) {
        if (!kek_hook_registry_add(registry, &descriptors[i])) {
            return 0;
        }
    }
    return 1;
}

void kek_hook_registry_attach(KekHookRegistry* registry) {
    if (!registry || !registry->runtime || registry->attached) {
        return;
    }

    KekEventDispatcher* dispatcher = kek_runtime_events(registry->runtime);
    for (size_t i = 0; i < KEK_EVENT_TYPE_COUNT; i++) {
        if (!kek_event_subscribe(dispatcher, (KekEventType)i,
                                 hook_registry_event_handler, registry)) {
            for (size_t j = 0; j < i; j++) {
                kek_event_unsubscribe(dispatcher, (KekEventType)j,
                                      hook_registry_event_handler, registry);
            }
            return;
        }
    }
    registry->attached = 1;
}

void kek_hook_registry_detach(KekHookRegistry* registry) {
    if (!registry || !registry->runtime || !registry->attached) {
        return;
    }

    KekEventDispatcher* dispatcher = kek_runtime_events(registry->runtime);
    for (size_t i = 0; i < KEK_EVENT_TYPE_COUNT; i++) {
        kek_event_unsubscribe(dispatcher, (KekEventType)i, hook_registry_event_handler,
                              registry);
    }
    registry->attached = 0;
}

static int hook_matches_changed_fields(const KekHookDescriptor* descriptor,
                                       const KekEvent* event) {
    if (!descriptor || descriptor->trigger_fields == KEK_EVENT_CHANGED_FIELDS_NONE) {
        return 1;
    }
    if (!event || event->changed_fields == KEK_EVENT_CHANGED_FIELDS_UNKNOWN) {
        return 1;
    }
    return (descriptor->trigger_fields & event->changed_fields) != 0;
}

int kek_hook_registry_dispatch(KekHookRegistry* registry, const KekEvent* event) {
    if (!registry || !event) {
        return 0;
    }

    for (size_t i = 0; i < registry->descriptor_count; i++) {
        const KekHookDescriptor* descriptor = &registry->descriptors[i];
        if (!descriptor || descriptor->event_type != event->type) {
            continue;
        }
        if (descriptor->state_type_id != KEK_HOOK_ANY_STATE &&
            descriptor->state_type_id != event->state_type_id) {
            continue;
        }
        if (descriptor->state_slot_id != KEK_HOOK_ANY_SLOT &&
            descriptor->state_slot_id != event->state_slot_id) {
            continue;
        }
        if (!hook_matches_changed_fields(descriptor, event)) {
            continue;
        }

        KekHookContext context;
        context.runtime = registry->runtime;
        context.state_store = registry->state_store;
        context.event = event;
        context.app_context = registry->app_context;
        KekStateStoreTransaction state_transaction;
        KekEventTransaction event_transaction;
        if (!kek_state_store_transaction_begin(registry->state_store,
                                               &state_transaction) ||
            !kek_event_transaction_begin(kek_runtime_events(registry->runtime),
                                         &event_transaction)) {
            kek_state_store_transaction_rollback(&state_transaction);
            return 0;
        }
        KekStateStoreHookExecution previous_hook;
        kek_state_store_begin_hook(registry->state_store, descriptor,
                                   event->state_slot_id, &previous_hook);
        if (!descriptor->run) {
            kek_state_store_end_hook(registry->state_store, &previous_hook);
            kek_event_transaction_rollback(&event_transaction);
            kek_state_store_transaction_rollback(&state_transaction);
            return 0;
        }
        uint64_t hook_start = kek_trace_enabled(registry->runtime)
                                  ? kek_trace_now_ns()
                                  : 0;
        uint64_t wait_ns = 0;
        if (hook_start != 0 && event->trace_published_ns != 0 &&
            hook_start >= event->trace_published_ns) {
            wait_ns = hook_start - event->trace_published_ns;
        }
        int ok = descriptor->run(&context);
        if (kek_trace_enabled(registry->runtime)) {
            uint64_t hook_end = kek_trace_now_ns();
            kek_trace_record_hook(registry->runtime, descriptor, event, wait_ns,
                                  hook_end - hook_start, ok);
        }
        kek_state_store_end_hook(registry->state_store, &previous_hook);
        if (!ok) {
            kek_event_transaction_rollback(&event_transaction);
            kek_state_store_transaction_rollback(&state_transaction);
            return 0;
        }
        kek_event_transaction_commit(&event_transaction);
        kek_state_store_transaction_commit(&state_transaction);
    }
    return 1;
}

const void* kek_hook_event_state(const KekHookContext* context, size_t* size) {
    if (size) {
        *size = 0;
    }
    if (!context || !context->event || !context->event->has_state_snapshot) {
        return NULL;
    }
    if (size) {
        *size = context->event->state_snapshot_size;
    }
    return context->event->state_snapshot.data;
}

#ifdef KEK_HOOK_DYNAMIC
static char* hook_copy_path(const char* path) {
    if (!path) {
        return NULL;
    }

    size_t length = strlen(path);
    char* copy = (char*)malloc(length + 1);
    if (!copy) {
        return NULL;
    }
    memcpy(copy, path, length + 1);
    return copy;
}

static char* hook_shadow_path(const char* path, unsigned long generation) {
    if (!path) {
        return NULL;
    }

    int needed = snprintf(NULL, 0, "%s.kek-%ld-%lu", path, (long)getpid(), generation);
    if (needed < 0) {
        return NULL;
    }
    char* shadow_path = (char*)malloc((size_t)needed + 1);
    if (!shadow_path) {
        return NULL;
    }
    snprintf(shadow_path, (size_t)needed + 1, "%s.kek-%ld-%lu", path, (long)getpid(),
             generation);
    return shadow_path;
}

static int hook_copy_library_file(const char* source_path, const char* target_path) {
    if (!source_path || !target_path) {
        return 0;
    }

    FILE* source = fopen(source_path, "rb");
    if (!source) {
        return 0;
    }
    FILE* target = fopen(target_path, "wb");
    if (!target) {
        fclose(source);
        return 0;
    }

    unsigned char buffer[8192];
    int ok = 1;
    for (;;) {
        size_t read_count = fread(buffer, 1, sizeof(buffer), source);
        if (read_count > 0 && fwrite(buffer, 1, read_count, target) != read_count) {
            ok = 0;
            break;
        }
        if (read_count < sizeof(buffer)) {
            if (ferror(source)) {
                ok = 0;
            }
            break;
        }
    }
    if (fclose(target) != 0) {
        ok = 0;
    }
    fclose(source);
    if (!ok) {
        remove(target_path);
    }
    return ok;
}

static int hook_registry_resolve_library(KekHookRegistry* registry, void* library,
                                         KekHookFn resolved[KEK_HOOK_MAX_DESCRIPTORS]) {
    if (!registry || !library || !resolved) {
        return 0;
    }

    for (size_t i = 0; i < registry->descriptor_count; i++) {
        const char* name = registry->descriptors[i].name;
        if (!name) {
            return 0;
        }
        void* symbol = dlsym(library, name);
        if (!symbol) {
            return 0;
        }
        resolved[i] = (KekHookFn)symbol;
    }
    return 1;
}

int kek_hook_registry_load_library(KekHookRegistry* registry, const char* path) {
    if (!registry || !path) {
        return 0;
    }

    unsigned long generation = registry->dynamic_generation + 1;
    char* path_copy = hook_copy_path(path);
    if (!path_copy) {
        return 0;
    }
    char* loaded_path = hook_shadow_path(path, generation);
    if (!loaded_path) {
        free(path_copy);
        return 0;
    }
    if (!hook_copy_library_file(path, loaded_path)) {
        free(loaded_path);
        free(path_copy);
        return 0;
    }

    void* library = dlopen(loaded_path, RTLD_NOW);
    if (!library) {
        remove(loaded_path);
        free(loaded_path);
        free(path_copy);
        return 0;
    }

    KekHookFn resolved[KEK_HOOK_MAX_DESCRIPTORS] = {0};
    if (!hook_registry_resolve_library(registry, library, resolved)) {
        dlclose(library);
        remove(loaded_path);
        free(loaded_path);
        free(path_copy);
        return 0;
    }

    void* old_library = registry->dynamic_library;
    char* old_path = registry->dynamic_library_path;
    char* old_loaded_path = registry->dynamic_loaded_path;
    for (size_t i = 0; i < registry->descriptor_count; i++) {
        registry->descriptors[i].run = resolved[i];
    }
    registry->dynamic_library = library;
    registry->dynamic_library_path = path_copy;
    registry->dynamic_loaded_path = loaded_path;
    registry->dynamic_generation = generation;

    if (old_library) {
        dlclose(old_library);
    }
    if (old_loaded_path) {
        remove(old_loaded_path);
    }
    free(old_path);
    free(old_loaded_path);
    return 1;
}

int kek_hook_registry_reload_library(KekHookRegistry* registry) {
    if (!registry || !registry->dynamic_library_path) {
        return 0;
    }
    return kek_hook_registry_load_library(registry, registry->dynamic_library_path);
}

void kek_hook_registry_unload_library(KekHookRegistry* registry) {
    if (!registry) {
        return;
    }
    if (registry->dynamic_library) {
        dlclose(registry->dynamic_library);
    }
    if (registry->dynamic_loaded_path) {
        remove(registry->dynamic_loaded_path);
    }
    free(registry->dynamic_library_path);
    free(registry->dynamic_loaded_path);
    registry->dynamic_library = NULL;
    registry->dynamic_library_path = NULL;
    registry->dynamic_loaded_path = NULL;
}
#endif
