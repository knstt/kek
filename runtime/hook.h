#ifndef KEK_RUNTIME_HOOK_H
#define KEK_RUNTIME_HOOK_H

#include <stddef.h>

#include "event.h"

#define KEK_HOOK_MAX_DESCRIPTORS 64
#define KEK_HOOK_ANY_STATE ((size_t)-1)
#define KEK_HOOK_ANY_SLOT ((size_t)-1)
#define KEK_HOOK_UNRESOLVED_SLOT ((size_t)-2)

struct KekRuntime;
struct KekStateStore;

typedef struct KekHookContext {
    struct KekRuntime* runtime;
    struct KekStateStore* state_store;
    const KekEvent* event;
    void* app_context;
} KekHookContext;

typedef int (*KekHookFn)(KekHookContext* context);

typedef struct KekHookDescriptor {
    const char* name;
    KekEventType event_type;
    size_t state_type_id;
    size_t state_slot_id;
    uint64_t trigger_fields;
    const size_t* reads;
    size_t read_count;
    const size_t* writes;
    size_t write_count;
    KekHookFn run;
} KekHookDescriptor;

typedef struct KekHookRegistry {
    struct KekRuntime* runtime;
    struct KekStateStore* state_store;
    void* app_context;
    const KekHookDescriptor* descriptors[KEK_HOOK_MAX_DESCRIPTORS];
    size_t descriptor_count;
    int attached;
} KekHookRegistry;

void kek_hook_registry_init(KekHookRegistry* registry, struct KekRuntime* runtime,
                            struct KekStateStore* state_store, void* app_context);
int kek_hook_registry_add(KekHookRegistry* registry,
                          const KekHookDescriptor* descriptor);
int kek_hook_registry_add_many(KekHookRegistry* registry,
                               const KekHookDescriptor* descriptors,
                               size_t descriptor_count);
void kek_hook_registry_attach(KekHookRegistry* registry);
void kek_hook_registry_detach(KekHookRegistry* registry);
int kek_hook_registry_dispatch(KekHookRegistry* registry, const KekEvent* event);
const void* kek_hook_event_state(const KekHookContext* context, size_t* size);

#endif
