#include "state_storage.h"

#include <stdlib.h>
#include <string.h>

#include "trace.h"

int kek_state_storage_init(KekStateStorage* storage, KekRuntime* runtime,
                           const void* initial_state, size_t state_size,
                           KekStateStorageCheckFn check) {
    if (!storage || !runtime || !initial_state || state_size == 0 || !check) {
        return 0;
    }

    memset(storage, 0, sizeof(*storage));
    storage->runtime = runtime;
    storage->state_size = state_size;
    storage->buffers[0] = (unsigned char*)kek_trace_malloc(runtime, state_size);
    storage->buffers[1] = (unsigned char*)kek_trace_malloc(runtime, state_size);
    if (!storage->buffers[0] || !storage->buffers[1]) {
        kek_state_storage_destroy(storage);
        return 0;
    }

    uint64_t copy_start = kek_trace_enabled(runtime) ? kek_trace_now_ns() : 0;
    memcpy(storage->buffers[0], initial_state, state_size);
    memcpy(storage->buffers[1], initial_state, state_size);
    if (kek_trace_enabled(runtime)) {
        kek_trace_record_runtime_metric(runtime,
                                        KEK_TRACE_METRIC_STATE_STORAGE_INIT_COPY,
                                        kek_trace_now_ns() - copy_start);
    }
    uint64_t check_start = kek_trace_enabled(runtime) ? kek_trace_now_ns() : 0;
    if (!check(storage->buffers[0])) {
        if (kek_trace_enabled(runtime)) {
            kek_trace_record_runtime_metric(
                runtime, KEK_TRACE_METRIC_STATE_STORAGE_VALIDATION,
                kek_trace_now_ns() - check_start);
        }
        kek_state_storage_destroy(storage);
        return 0;
    }
    if (kek_trace_enabled(runtime)) {
        kek_trace_record_runtime_metric(runtime,
                                        KEK_TRACE_METRIC_STATE_STORAGE_VALIDATION,
                                        kek_trace_now_ns() - check_start);
    }

    storage->active_index = 0;
    storage->check = check;
    return 1;
}

void kek_state_storage_destroy(KekStateStorage* storage) {
    if (!storage) {
        return;
    }

    kek_trace_free(storage->runtime, storage->buffers[0], storage->state_size);
    kek_trace_free(storage->runtime, storage->buffers[1], storage->state_size);
    memset(storage, 0, sizeof(*storage));
}

void* kek_state_storage_current(KekStateStorage* storage) {
    if (!storage) {
        return NULL;
    }
    return storage->buffers[storage->active_index];
}

const void* kek_state_storage_current_const(const KekStateStorage* storage) {
    if (!storage) {
        return NULL;
    }
    return storage->buffers[storage->active_index];
}

void* kek_state_storage_copy(KekStateStorage* storage, size_t index) {
    if (!storage || index >= 2) {
        return NULL;
    }
    return storage->buffers[index];
}

const void* kek_state_storage_copy_const(const KekStateStorage* storage, size_t index) {
    if (!storage || index >= 2) {
        return NULL;
    }
    return storage->buffers[index];
}

int kek_state_storage_update(KekStateStorage* storage,
                              KekStateStorageUpdateFn update, void* context) {
    if (!storage || !storage->runtime || !storage->check || !update) {
        return 0;
    }
    if (kek_event_capacity_remaining(kek_runtime_events(storage->runtime)) == 0) {
        return 0;
    }

    size_t inactive_index = storage->active_index == 0 ? 1u : 0u;
    void* current = storage->buffers[storage->active_index];
    void* draft = storage->buffers[inactive_index];
    uint64_t copy_start = kek_trace_enabled(storage->runtime) ? kek_trace_now_ns() : 0;
    memcpy(draft, current, storage->state_size);
    if (kek_trace_enabled(storage->runtime)) {
        kek_trace_record_runtime_metric(
            storage->runtime, KEK_TRACE_METRIC_STATE_STORAGE_DRAFT_COPY,
            kek_trace_now_ns() - copy_start);
    }

    uint64_t update_start = kek_trace_enabled(storage->runtime) ? kek_trace_now_ns() : 0;
    update(draft, context);
    if (kek_trace_enabled(storage->runtime)) {
        kek_trace_record_runtime_metric(
            storage->runtime, KEK_TRACE_METRIC_STATE_STORAGE_UPDATE_CALLBACK,
            kek_trace_now_ns() - update_start);
    }
    uint64_t check_start = kek_trace_enabled(storage->runtime) ? kek_trace_now_ns() : 0;
    if (!storage->check(draft)) {
        if (kek_trace_enabled(storage->runtime)) {
            kek_trace_record_runtime_metric(
                storage->runtime, KEK_TRACE_METRIC_STATE_STORAGE_VALIDATION,
                kek_trace_now_ns() - check_start);
        }
        uint64_t rollback_start =
            kek_trace_enabled(storage->runtime) ? kek_trace_now_ns() : 0;
        memcpy(draft, current, storage->state_size);
        if (kek_trace_enabled(storage->runtime)) {
            kek_trace_record_runtime_metric(
                storage->runtime, KEK_TRACE_METRIC_STATE_STORAGE_ROLLBACK_COPY,
                kek_trace_now_ns() - rollback_start);
        }
        return 0;
    }
    if (kek_trace_enabled(storage->runtime)) {
        kek_trace_record_runtime_metric(
            storage->runtime, KEK_TRACE_METRIC_STATE_STORAGE_VALIDATION,
            kek_trace_now_ns() - check_start);
    }

    storage->active_index = inactive_index;
    if (kek_runtime_publish_state_changed(storage->runtime,
                                          kek_state_storage_current(storage))) {
        return 1;
    }

    storage->active_index = inactive_index == 0 ? 1u : 0u;
    uint64_t rollback_start =
        kek_trace_enabled(storage->runtime) ? kek_trace_now_ns() : 0;
    memcpy(draft, current, storage->state_size);
    if (kek_trace_enabled(storage->runtime)) {
        kek_trace_record_runtime_metric(
            storage->runtime, KEK_TRACE_METRIC_STATE_STORAGE_ROLLBACK_COPY,
            kek_trace_now_ns() - rollback_start);
    }
    return 0;
}
