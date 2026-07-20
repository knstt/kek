#include "hook.h"

#include <stdlib.h>
#include <string.h>

#ifdef KEK_HOOK_DYNAMIC
#include <dlfcn.h>
#include <stdio.h>
#include <unistd.h>
#endif

#include "runtime.h"
#include "state_storage.h"
#include "trace.h"

static int hook_registry_event_handler(const KekEvent* event, void* context) {
    return kek_hook_registry_dispatch((KekHookRegistry*)context, event);
}

static void hook_registry_clear_index(KekHookRegistry* registry) {
    if (!registry) {
        return;
    }

    for (size_t i = 0; i < KEK_HOOK_MAX_DESCRIPTORS; i++) {
        registry->next_descriptor_indices[i] = KEK_HOOK_INVALID_DESCRIPTOR;
    }
    for (size_t i = 0; i < KEK_EVENT_TYPE_COUNT; i++) {
        registry->event_buckets[i].first_descriptor_index =
            KEK_HOOK_INVALID_DESCRIPTOR;
        registry->event_buckets[i].last_descriptor_index =
            KEK_HOOK_INVALID_DESCRIPTOR;
        registry->event_buckets[i].count = 0;
        registry->state_bucket_counts[i] = 0;
    }
}

static KekHookBucket* hook_registry_find_state_bucket(KekHookRegistry* registry,
                                                      KekEventType event_type,
                                                      size_t state_type_id,
                                                      int create) {
    if (!registry || event_type < 0 || event_type >= KEK_EVENT_TYPE_COUNT) {
        return NULL;
    }

    size_t* bucket_count = &registry->state_bucket_counts[event_type];
    KekHookStateBucket* buckets = registry->state_buckets[event_type];
    for (size_t i = 0; i < *bucket_count; i++) {
        if (buckets[i].state_type_id == state_type_id) {
            return &buckets[i].bucket;
        }
    }
    if (!create || *bucket_count >= KEK_HOOK_MAX_STATE_BUCKETS) {
        return NULL;
    }

    KekHookStateBucket* bucket = &buckets[(*bucket_count)++];
    bucket->state_type_id = state_type_id;
    bucket->bucket.first_descriptor_index = KEK_HOOK_INVALID_DESCRIPTOR;
    bucket->bucket.last_descriptor_index = KEK_HOOK_INVALID_DESCRIPTOR;
    bucket->bucket.count = 0;
    return &bucket->bucket;
}

static int hook_registry_append_to_bucket(KekHookRegistry* registry,
                                          KekHookBucket* bucket,
                                          size_t descriptor_index) {
    if (!registry || !bucket || descriptor_index >= registry->descriptor_count ||
        bucket->count >= KEK_HOOK_MAX_DESCRIPTORS) {
        return 0;
    }

    registry->next_descriptor_indices[descriptor_index] =
        KEK_HOOK_INVALID_DESCRIPTOR;
    if (bucket->last_descriptor_index != KEK_HOOK_INVALID_DESCRIPTOR) {
        registry->next_descriptor_indices[bucket->last_descriptor_index] =
            descriptor_index;
    } else {
        bucket->first_descriptor_index = descriptor_index;
    }
    bucket->last_descriptor_index = descriptor_index;
    bucket->count++;
    return 1;
}

static int hook_registry_index_descriptor(KekHookRegistry* registry,
                                          size_t descriptor_index) {
    if (!registry || descriptor_index >= registry->descriptor_count) {
        return 0;
    }

    const KekHookDescriptor* descriptor = &registry->descriptors[descriptor_index];
    if (descriptor->event_type < 0 ||
        descriptor->event_type >= KEK_EVENT_TYPE_COUNT) {
        return 0;
    }

    KekHookBucket* bucket = descriptor->state_type_id == KEK_HOOK_ANY_STATE
                                ? &registry->event_buckets[descriptor->event_type]
                                : hook_registry_find_state_bucket(
                                      registry, descriptor->event_type,
                                      descriptor->state_type_id, 1);
    if (!bucket) {
        return 0;
    }
    return hook_registry_append_to_bucket(registry, bucket, descriptor_index);
}

static int hook_registry_rebuild_index(KekHookRegistry* registry) {
    if (!registry) {
        return 0;
    }

    hook_registry_clear_index(registry);
    for (size_t i = 0; i < registry->descriptor_count; i++) {
        if (!hook_registry_index_descriptor(registry, i)) {
            hook_registry_clear_index(registry);
            return 0;
        }
    }
    return 1;
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
    hook_registry_clear_index(registry);
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

    size_t descriptor_index = registry->descriptor_count;
    registry->descriptors[descriptor_index] = *descriptor;
    registry->descriptor_count++;
    if (!hook_registry_index_descriptor(registry, descriptor_index)) {
        registry->descriptor_count--;
        return 0;
    }
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
    if (!hook_registry_rebuild_index(registry)) {
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

typedef struct KekHookDispatchJob {
    KekHookRegistry* registry;
    KekEvent event;
    const KekHookDescriptor* descriptor;
    int ok;
    int quit_requested;
    uint64_t wait_ns;
    uint64_t run_ns;
    uint64_t base_versions[KEK_STATE_STORE_MAX_SLOTS];
    KekRuntime runtime;
    KekStateStore state_store;
} KekHookDispatchJob;

static int hook_descriptor_has_dependency(const size_t* dependencies,
                                          size_t dependency_count,
                                          size_t state_type_id) {
    if (!dependencies) {
        return 0;
    }
    for (size_t i = 0; i < dependency_count; i++) {
        if (dependencies[i] == state_type_id ||
            dependencies[i] == KEK_HOOK_ANY_STATE) {
            return 1;
        }
    }
    return 0;
}

static int hook_descriptor_can_run_parallel(const KekHookDescriptor* descriptor) {
    if (!descriptor || !descriptor->run ||
        (descriptor->scheduling_flags & KEK_HOOK_SCHEDULING_OPAQUE)) {
        return 0;
    }
    if (descriptor->access_count > 0 && descriptor->accesses) {
        int has_access = 0;
        for (size_t i = 0; i < descriptor->access_count; i++) {
            const KekHookAccess* access = &descriptor->accesses[i];
            has_access = 1;
            if (access->mode == KEK_HOOK_ACCESS_CREATE ||
                access->mode == KEK_HOOK_ACCESS_DELETE) {
                return 0;
            }
            if (access->mode == KEK_HOOK_ACCESS_WRITE &&
                (!(descriptor->scheduling_flags &
                   KEK_HOOK_SCHEDULING_ALLOW_PARALLEL_WRITES) ||
                 access->scope != KEK_HOOK_ACCESS_SCOPE_EXACT_SLOT ||
                 access->state_slot_id == KEK_HOOK_ANY_SLOT ||
                 access->state_slot_id == KEK_HOOK_UNRESOLVED_SLOT)) {
                return 0;
            }
        }
        return has_access;
    }
    if (descriptor->write_count != 0 || descriptor->read_count == 0 ||
        !descriptor->reads) {
        return 0;
    }
    for (size_t i = 0; i < descriptor->read_count; i++) {
        if (descriptor->reads[i] == KEK_HOOK_ANY_STATE) {
            return 0;
        }
    }
    return 1;
}

static int hook_descriptor_has_precise_write(const KekHookDescriptor* descriptor) {
    if (!descriptor || !descriptor->accesses) {
        return 0;
    }
    for (size_t i = 0; i < descriptor->access_count; i++) {
        if (descriptor->accesses[i].mode == KEK_HOOK_ACCESS_WRITE) {
            return 1;
        }
    }
    return 0;
}

static int hook_descriptor_has_field_merge_write(const KekHookDescriptor* descriptor) {
    return hook_descriptor_has_precise_write(descriptor) &&
           descriptor &&
           (descriptor->scheduling_flags & KEK_HOOK_SCHEDULING_FIELD_MERGE_SAFE);
}

static int hook_access_is_write_like(const KekHookAccess* access) {
    return access && (access->mode == KEK_HOOK_ACCESS_WRITE ||
                      access->mode == KEK_HOOK_ACCESS_CREATE ||
                      access->mode == KEK_HOOK_ACCESS_DELETE);
}

static int hook_access_fields_known(uint64_t fields) {
    return fields != KEK_EVENT_CHANGED_FIELDS_UNKNOWN;
}

static int hook_access_scopes_overlap(const KekHookAccess* left,
                                      const KekHookAccess* right) {
    if (!left || !right || left->state_type_id != right->state_type_id) {
        return 0;
    }
    if (left->scope == KEK_HOOK_ACCESS_SCOPE_ANY ||
        right->scope == KEK_HOOK_ACCESS_SCOPE_ANY) {
        return 1;
    }
    if (left->scope == KEK_HOOK_ACCESS_SCOPE_EXACT_SLOT &&
        right->scope == KEK_HOOK_ACCESS_SCOPE_EXACT_SLOT) {
        return left->state_slot_id == right->state_slot_id;
    }
    if (left->scope == KEK_HOOK_ACCESS_SCOPE_DYNAMIC ||
        right->scope == KEK_HOOK_ACCESS_SCOPE_DYNAMIC) {
        return left->scope == right->scope;
    }
    return 1;
}

static int hook_access_pair_conflicts(const KekHookDescriptor* left_descriptor,
                                      const KekHookAccess* left,
                                      const KekHookDescriptor* right_descriptor,
                                      const KekHookAccess* right) {
    if (!left || !right || !hook_access_scopes_overlap(left, right)) {
        return 0;
    }
    int left_writes = hook_access_is_write_like(left);
    int right_writes = hook_access_is_write_like(right);
    if (!left_writes && !right_writes) {
        return 0;
    }
    if (left->mode == KEK_HOOK_ACCESS_DELETE ||
        right->mode == KEK_HOOK_ACCESS_DELETE ||
        left->mode == KEK_HOOK_ACCESS_CREATE ||
        right->mode == KEK_HOOK_ACCESS_CREATE) {
        return 1;
    }
    if (left_writes && right_writes &&
        left->scope == KEK_HOOK_ACCESS_SCOPE_EXACT_SLOT &&
        right->scope == KEK_HOOK_ACCESS_SCOPE_EXACT_SLOT &&
        left->state_slot_id == right->state_slot_id &&
        hook_access_fields_known(left->fields) &&
        hook_access_fields_known(right->fields) &&
        (left->fields & right->fields) == 0 &&
        (left_descriptor->scheduling_flags &
         KEK_HOOK_SCHEDULING_FIELD_MERGE_SAFE) &&
        (right_descriptor->scheduling_flags &
         KEK_HOOK_SCHEDULING_FIELD_MERGE_SAFE)) {
        return 0;
    }
    return 1;
}

static int hook_descriptors_access_conflict(const KekHookDescriptor* left,
                                            const KekHookDescriptor* right) {
    if (!left || !right || !left->accesses || !right->accesses) {
        return 1;
    }
    for (size_t i = 0; i < left->access_count; i++) {
        for (size_t j = 0; j < right->access_count; j++) {
            if (hook_access_pair_conflicts(left, &left->accesses[i], right,
                                           &right->accesses[j])) {
                return 1;
            }
        }
    }
    return 0;
}

static int hook_descriptors_conflict(const KekHookDescriptor* left,
                                     const KekHookDescriptor* right) {
    if (!left || !right) {
        return 1;
    }
    if (!hook_descriptor_can_run_parallel(left) ||
        !hook_descriptor_can_run_parallel(right)) {
        return 1;
    }
    if ((left->access_count > 0 || right->access_count > 0) &&
        !(left->access_count > 0 && right->access_count > 0 &&
          left->accesses && right->accesses)) {
        return 1;
    }
    if (left->access_count > 0 && right->access_count > 0) {
        return hook_descriptors_access_conflict(left, right);
    }
    for (size_t i = 0; i < left->write_count; i++) {
        if (hook_descriptor_has_dependency(right->reads, right->read_count,
                                           left->writes[i]) ||
            hook_descriptor_has_dependency(right->writes, right->write_count,
                                           left->writes[i])) {
            return 1;
        }
    }
    for (size_t i = 0; i < right->write_count; i++) {
        if (hook_descriptor_has_dependency(left->reads, left->read_count,
                                           right->writes[i]) ||
            hook_descriptor_has_dependency(left->writes, left->write_count,
                                           right->writes[i])) {
            return 1;
        }
    }
    return 0;
}

static void hook_job_destroy(KekHookDispatchJob* job) {
    if (!job) {
        return;
    }
    kek_state_store_destroy(&job->state_store);
}

static int hook_job_init(KekHookDispatchJob* job, KekHookRegistry* registry,
                         const KekEvent* event,
                         const KekHookDescriptor* descriptor) {
    if (!job || !registry || !registry->runtime || !registry->state_store ||
        !event || !descriptor) {
        return 0;
    }

    memset(job, 0, sizeof(*job));
    job->registry = registry;
    job->event = *event;
    job->descriptor = descriptor;

    memset(&job->runtime, 0, sizeof(job->runtime));
    kek_event_dispatcher_init(&job->runtime.events);
    job->runtime.events.runtime = &job->runtime;
    job->runtime.quit_requested = registry->runtime->quit_requested;

    memset(&job->state_store, 0, sizeof(job->state_store));
    job->state_store.runtime = &job->runtime;
    job->state_store.slot_count = registry->state_store->slot_count;
    job->state_store.active_hook.trigger_state_slot = KEK_STATE_INVALID_ID;

    uint64_t clone_start = kek_trace_enabled(registry->runtime) ? kek_trace_now_ns() : 0;
    uint64_t clone_bytes = 0;
    for (size_t i = 0; i < registry->state_store->slot_count; i++) {
        const KekStateSlot* source = &registry->state_store->slots[i];
        KekStateSlot* target = &job->state_store.slots[i];
        job->base_versions[i] = source->version;
        target->descriptor = source->descriptor;
        target->active_index = 0;
        target->version = source->version;
        target->in_use = source->in_use;
        if (!source->in_use || !source->descriptor) {
            continue;
        }

        size_t size = source->descriptor->size;
        clone_bytes += size * 2u;
        target->buffers[0] = (unsigned char*)malloc(size);
        target->buffers[1] = (unsigned char*)malloc(size);
        if (!target->buffers[0] || !target->buffers[1]) {
            hook_job_destroy(job);
            return 0;
        }
        const void* current = source->buffers[source->active_index];
        memcpy(target->buffers[0], current, size);
        memcpy(target->buffers[1], current, size);
    }
    if (clone_start != 0) {
        kek_trace_record_runtime_metric(registry->runtime,
                                        KEK_TRACE_METRIC_HOOK_WORKER_CLONE,
                                        kek_trace_now_ns() - clone_start);
        kek_trace_record_runtime_metric(
            registry->runtime, KEK_TRACE_METRIC_HOOK_WORKER_CLONE_BYTES,
            clone_bytes);
    }
    return 1;
}

static void hook_job_publish_events(KekHookDispatchJob* job) {
    if (!job || !job->registry || !job->registry->runtime) {
        return;
    }
    KekEventDispatcher* dispatcher = &job->runtime.events;
    if (kek_event_capacity_remaining(kek_runtime_events(job->registry->runtime)) <
        dispatcher->queue_size) {
        job->ok = 0;
        return;
    }
    uint64_t replay_start =
        kek_trace_enabled(job->registry->runtime) ? kek_trace_now_ns() : 0;
    while (dispatcher->queue_size > 0) {
        KekEvent event = dispatcher->queue[dispatcher->queue_start];
        dispatcher->queue_start =
            (dispatcher->queue_start + 1) % KEK_EVENT_QUEUE_CAPACITY;
        dispatcher->queue_size--;
        if (event.type == KEK_EVENT_STATE_CHANGED ||
            event.type == KEK_EVENT_STATE_CREATED ||
            event.type == KEK_EVENT_STATE_DELETED) {
            if (event.state_slot_id < job->registry->state_store->slot_count) {
                KekStateSlot* live_slot =
                    &job->registry->state_store->slots[event.state_slot_id];
                if (live_slot->in_use && live_slot->descriptor) {
                    const void* source =
                        live_slot->buffers[live_slot->active_index];
                    event.source = (void*)source;
                    event.state_version = live_slot->version;
                    if (live_slot->descriptor->size <=
                        KEK_EVENT_STATE_SNAPSHOT_CAPACITY) {
                        memcpy(event.state_snapshot.data, source,
                               live_slot->descriptor->size);
                        event.state_snapshot_size = live_slot->descriptor->size;
                        event.has_state_snapshot = 1;
                    }
                }
            }
        }
        if (!kek_event_publish(kek_runtime_events(job->registry->runtime), &event)) {
            job->ok = 0;
            return;
        }
    }
    if (replay_start != 0) {
        kek_trace_record_runtime_metric(job->registry->runtime,
                                        KEK_TRACE_METRIC_HOOK_WORKER_EVENT_REPLAY,
                                        kek_trace_now_ns() - replay_start);
    }
}

static void hook_job_run(void* context) {
    KekHookDispatchJob* job = (KekHookDispatchJob*)context;
    if (!job || !job->descriptor || !job->descriptor->run) {
        return;
    }

    KekHookContext hook_context;
    hook_context.runtime = &job->runtime;
    hook_context.state_store = &job->state_store;
    hook_context.event = &job->event;
    hook_context.app_context = job->registry->app_context;

    KekStateStoreTransaction state_transaction;
    KekEventTransaction event_transaction;
    if (!kek_state_store_transaction_begin_for_hook(
            &job->state_store, &state_transaction, job->descriptor) ||
        !kek_event_transaction_begin(kek_runtime_events(&job->runtime),
                                     &event_transaction)) {
        kek_state_store_transaction_rollback(&state_transaction);
        job->ok = 0;
        return;
    }

    KekStateStoreHookExecution previous_hook;
    kek_state_store_begin_hook(&job->state_store, job->descriptor,
                               job->event.state_slot_id, &previous_hook);
    uint64_t hook_start = kek_trace_now_ns();
    if (job->event.trace_published_ns != 0 &&
        hook_start >= job->event.trace_published_ns) {
        job->wait_ns = hook_start - job->event.trace_published_ns;
    }
    job->ok = job->descriptor->run(&hook_context);
    uint64_t hook_end = kek_trace_now_ns();
    job->run_ns = hook_end >= hook_start ? hook_end - hook_start : 0;
    job->quit_requested = job->runtime.quit_requested;

    kek_state_store_end_hook(&job->state_store, &previous_hook);
    if (!job->ok) {
        kek_event_transaction_rollback(&event_transaction);
        kek_state_store_transaction_rollback(&state_transaction);
        return;
    }
    kek_event_transaction_commit(&event_transaction);
    kek_state_store_transaction_commit(&state_transaction);
}

static KekStateSlot* hook_store_live_slot(KekStateStore* store, size_t slot_id) {
    if (!store || slot_id >= store->slot_count || !store->slots[slot_id].in_use) {
        return NULL;
    }
    return &store->slots[slot_id];
}

static int hook_job_apply_slot(KekHookDispatchJob* job, const KekHookAccess* access) {
    if (!job || !job->registry || !job->registry->state_store || !access ||
        access->scope != KEK_HOOK_ACCESS_SCOPE_EXACT_SLOT ||
        access->state_slot_id >= KEK_STATE_STORE_MAX_SLOTS) {
        return 0;
    }

    size_t slot_id = access->state_slot_id;
    KekStateSlot* target =
        hook_store_live_slot(job->registry->state_store, slot_id);
    KekStateSlot* source = hook_store_live_slot(&job->state_store, slot_id);
    if (!target || !source || !target->descriptor || !source->descriptor ||
        target->descriptor->type_id != source->descriptor->type_id) {
        return 0;
    }

    uint64_t base_version = job->base_versions[slot_id];
    if (source->version == base_version) {
        return 1;
    }
    uint64_t version_delta = source->version > base_version
                                 ? source->version - base_version
                                 : 1;
    int field_merge =
        (job->descriptor->scheduling_flags &
         KEK_HOOK_SCHEDULING_FIELD_MERGE_SAFE) &&
        access->fields != KEK_EVENT_CHANGED_FIELDS_UNKNOWN &&
        target->descriptor->merge_fields;
    if (!field_merge && target->version != base_version) {
        return 0;
    }

    size_t inactive_index = target->active_index == 0 ? 1u : 0u;
    const void* source_current = source->buffers[source->active_index];
    void* target_current = target->buffers[target->active_index];
    void* target_draft = target->buffers[inactive_index];
    memcpy(target_draft, target_current, target->descriptor->size);
    if (field_merge) {
        if (!target->descriptor->merge_fields(target_draft, source_current,
                                             access->fields)) {
            return 0;
        }
    } else {
        memcpy(target_draft, source_current, target->descriptor->size);
    }
    if (target->descriptor->check &&
        !target->descriptor->check(target_draft)) {
        memcpy(target_draft, target_current, target->descriptor->size);
        return 0;
    }

    target->active_index = inactive_index;
    target->version = field_merge ? target->version + version_delta
                                  : source->version;
    return 1;
}

static int hook_job_apply_state_changes(KekHookDispatchJob* job) {
    if (!job || !job->descriptor) {
        return 0;
    }
    if (job->descriptor->access_count == 0 || !job->descriptor->accesses) {
        return 1;
    }

    uint64_t apply_start =
        kek_trace_enabled(job->registry->runtime) ? kek_trace_now_ns() : 0;
    int ok = 1;
    size_t applied_slots[KEK_HOOK_MAX_DESCRIPTORS];
    size_t applied_count = 0;
    for (size_t i = 0; i < job->descriptor->access_count; i++) {
        const KekHookAccess* access = &job->descriptor->accesses[i];
        if (access->mode != KEK_HOOK_ACCESS_WRITE ||
            access->scope != KEK_HOOK_ACCESS_SCOPE_EXACT_SLOT) {
            continue;
        }
        int already_applied = 0;
        for (size_t j = 0; j < applied_count; j++) {
            if (applied_slots[j] == access->state_slot_id) {
                already_applied = 1;
                break;
            }
        }
        if (already_applied) {
            continue;
        }
        if (!hook_job_apply_slot(job, access)) {
            ok = 0;
            break;
        }
        if (applied_count < KEK_HOOK_MAX_DESCRIPTORS) {
            applied_slots[applied_count++] = access->state_slot_id;
        }
    }
    if (apply_start != 0) {
        kek_trace_record_runtime_metric(job->registry->runtime,
                                        KEK_TRACE_METRIC_HOOK_WORKER_APPLY,
                                        kek_trace_now_ns() - apply_start);
    }
    return ok;
}

static int hook_registry_dispatch_descriptor(KekHookRegistry* registry,
                                             const KekEvent* event,
                                             size_t descriptor_index) {
    if (!registry || !event || descriptor_index >= registry->descriptor_count) {
        return 0;
    }

    const KekHookDescriptor* descriptor = &registry->descriptors[descriptor_index];
    if (descriptor->state_slot_id != KEK_HOOK_ANY_SLOT &&
        descriptor->state_slot_id != event->state_slot_id) {
        return 1;
    }
    if (!hook_matches_changed_fields(descriptor, event)) {
        return 1;
    }
    kek_trace_count_runtime_metric(registry->runtime,
                                   KEK_TRACE_METRIC_HOOK_SERIAL_DESCRIPTOR);

    KekHookContext context;
    context.runtime = registry->runtime;
    context.state_store = registry->state_store;
    context.event = event;
    context.app_context = registry->app_context;
    KekStateStoreTransaction state_transaction;
    KekEventTransaction event_transaction;
    if (!kek_state_store_transaction_begin_for_hook(
            registry->state_store, &state_transaction, descriptor) ||
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
    return 1;
}

static int hook_registry_dispatch_parallel_wave(KekHookRegistry* registry,
                                                const KekEvent* event,
                                                const size_t* descriptor_indices,
                                                size_t descriptor_count) {
    if (!registry || !event || !descriptor_indices || descriptor_count == 0) {
        return 0;
    }
    if (descriptor_count == 1 || kek_runtime_thread_count(registry->runtime) <= 1) {
        kek_trace_count_runtime_metric(registry->runtime,
                                       KEK_TRACE_METRIC_HOOK_SERIAL_FALLBACK);
        for (size_t i = 0; i < descriptor_count; i++) {
            if (!hook_registry_dispatch_descriptor(registry, event,
                                                   descriptor_indices[i])) {
                return 0;
            }
        }
        return 1;
    }

    int has_write = 0;
    int has_field_merge = 0;
    for (size_t i = 0; i < descriptor_count; i++) {
        const KekHookDescriptor* descriptor =
            &registry->descriptors[descriptor_indices[i]];
        has_write = has_write || hook_descriptor_has_precise_write(descriptor);
        has_field_merge =
            has_field_merge || hook_descriptor_has_field_merge_write(descriptor);
    }
    kek_trace_record_runtime_metric(registry->runtime,
                                    KEK_TRACE_METRIC_HOOK_PARALLEL_WAVE,
                                    descriptor_count);
    if (has_write) {
        kek_trace_count_runtime_metric(registry->runtime,
                                       KEK_TRACE_METRIC_HOOK_PARALLEL_WRITE_WAVE);
    } else {
        kek_trace_count_runtime_metric(
            registry->runtime, KEK_TRACE_METRIC_HOOK_PARALLEL_READONLY_WAVE);
    }
    if (has_field_merge) {
        kek_trace_count_runtime_metric(registry->runtime,
                                       KEK_TRACE_METRIC_HOOK_FIELD_MERGE_WAVE);
    }

    KekHookDispatchJob* dispatch_jobs =
        (KekHookDispatchJob*)calloc(descriptor_count, sizeof(*dispatch_jobs));
    KekThreadPoolJob* pool_jobs =
        (KekThreadPoolJob*)calloc(descriptor_count, sizeof(*pool_jobs));
    if (!dispatch_jobs || !pool_jobs) {
        free(dispatch_jobs);
        free(pool_jobs);
        return 0;
    }

    for (size_t i = 0; i < descriptor_count; i++) {
        const KekHookDescriptor* descriptor =
            &registry->descriptors[descriptor_indices[i]];
        if (!hook_job_init(&dispatch_jobs[i], registry, event, descriptor)) {
            for (size_t j = 0; j < i; j++) {
                hook_job_destroy(&dispatch_jobs[j]);
            }
            free(dispatch_jobs);
            free(pool_jobs);
            return 0;
        }
        pool_jobs[i].run = hook_job_run;
        pool_jobs[i].context = &dispatch_jobs[i];
    }

    if (!kek_thread_pool_run(&registry->runtime->thread_pool, pool_jobs,
                             descriptor_count)) {
        for (size_t i = 0; i < descriptor_count; i++) {
            hook_job_destroy(&dispatch_jobs[i]);
        }
        free(dispatch_jobs);
        free(pool_jobs);
        return 0;
    }

    int ok = 1;
    for (size_t i = 0; i < descriptor_count; i++) {
        KekHookDispatchJob* job = &dispatch_jobs[i];
        if (kek_trace_enabled(registry->runtime)) {
            kek_trace_record_hook(registry->runtime, job->descriptor, event,
                                  job->wait_ns, job->run_ns, job->ok);
        }
        if (!job->ok) {
            ok = 0;
            break;
        }
        if (!hook_job_apply_state_changes(job)) {
            ok = 0;
            break;
        }
        hook_job_publish_events(job);
        if (!job->ok) {
            ok = 0;
            break;
        }
        if (job->quit_requested) {
            kek_runtime_request_quit(registry->runtime);
        }
    }

    for (size_t i = 0; i < descriptor_count; i++) {
        hook_job_destroy(&dispatch_jobs[i]);
    }
    free(dispatch_jobs);
    free(pool_jobs);
    return ok;
}

static int hook_registry_collect_bucket(KekHookRegistry* registry,
                                        const KekEvent* event,
                                        const KekHookBucket* bucket,
                                        size_t* matches,
                                        size_t* match_count) {
    if (!registry || !event || !bucket || !matches || !match_count) {
        return 0;
    }

    size_t descriptor_index = bucket->first_descriptor_index;
    for (size_t i = 0; i < bucket->count; i++) {
        if (descriptor_index >= registry->descriptor_count ||
            *match_count >= KEK_HOOK_MAX_DESCRIPTORS) {
            return 0;
        }
        const KekHookDescriptor* descriptor =
            &registry->descriptors[descriptor_index];
        if ((descriptor->state_slot_id == KEK_HOOK_ANY_SLOT ||
             descriptor->state_slot_id == event->state_slot_id) &&
            hook_matches_changed_fields(descriptor, event)) {
            matches[(*match_count)++] = descriptor_index;
        }
        descriptor_index = registry->next_descriptor_indices[descriptor_index];
    }
    return 1;
}

int kek_hook_registry_dispatch(KekHookRegistry* registry, const KekEvent* event) {
    if (!registry || !event) {
        return 0;
    }

    if (event->type < 0 || event->type >= KEK_EVENT_TYPE_COUNT) {
        return 1;
    }

    size_t matches[KEK_HOOK_MAX_DESCRIPTORS];
    size_t match_count = 0;
    if (!hook_registry_collect_bucket(registry, event,
                                      &registry->event_buckets[event->type],
                                      matches, &match_count)) {
        return 0;
    }

    const KekHookBucket* state_bucket =
        hook_registry_find_state_bucket(registry, event->type, event->state_type_id,
                                        0);
    if (state_bucket && !hook_registry_collect_bucket(registry, event, state_bucket,
                                                      matches, &match_count)) {
        return 0;
    }

    size_t cursor = 0;
    while (cursor < match_count) {
        const KekHookDescriptor* descriptor =
            &registry->descriptors[matches[cursor]];
        if (!hook_descriptor_can_run_parallel(descriptor)) {
            if (!hook_registry_dispatch_descriptor(registry, event,
                                                   matches[cursor])) {
                return 0;
            }
            cursor++;
            continue;
        }

        size_t wave[KEK_HOOK_MAX_DESCRIPTORS];
        size_t wave_count = 0;
        while (cursor < match_count) {
            const KekHookDescriptor* candidate =
                &registry->descriptors[matches[cursor]];
            if (!hook_descriptor_can_run_parallel(candidate)) {
                break;
            }
            int conflict = 0;
            for (size_t i = 0; i < wave_count; i++) {
                if (hook_descriptors_conflict(
                        &registry->descriptors[wave[i]], candidate)) {
                    conflict = 1;
                    break;
                }
            }
            if (conflict) {
                break;
            }
            wave[wave_count++] = matches[cursor++];
        }

        if (!hook_registry_dispatch_parallel_wave(registry, event, wave,
                                                  wave_count)) {
            return 0;
        }
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
