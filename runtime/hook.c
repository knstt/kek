#include "hook.h"

#include "runtime.h"
#include "state_storage.h"

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
        int ok = descriptor->run(&context);
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
