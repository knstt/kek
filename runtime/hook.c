#include "hook.h"

#include "runtime.h"
#include "state_storage.h"

static void hook_registry_event_handler(const KekEvent* event, void* context) {
    kek_hook_registry_dispatch((KekHookRegistry*)context, event);
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
}

int kek_hook_registry_add(KekHookRegistry* registry,
                          const KekHookDescriptor* descriptor) {
    if (!registry || !descriptor || !descriptor->run ||
        registry->descriptor_count >= KEK_HOOK_MAX_DESCRIPTORS) {
        return 0;
    }

    registry->descriptors[registry->descriptor_count++] = descriptor;
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

void kek_hook_registry_dispatch(KekHookRegistry* registry, const KekEvent* event) {
    if (!registry || !event) {
        return;
    }

    for (size_t i = 0; i < registry->descriptor_count; i++) {
        const KekHookDescriptor* descriptor = registry->descriptors[i];
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

        KekHookContext context;
        context.runtime = registry->runtime;
        context.state_store = registry->state_store;
        context.event = event;
        context.app_context = registry->app_context;
        KekStateStoreHookExecution previous_hook;
        kek_state_store_begin_hook(registry->state_store, descriptor,
                                   event->state_slot_id, &previous_hook);
        descriptor->run(&context);
        kek_state_store_end_hook(registry->state_store, &previous_hook);
    }
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
