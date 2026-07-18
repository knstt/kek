#include "state_storage.h"

#include <stdlib.h>
#include <string.h>

int kek_state_storage_init(KekStateStorage* storage, KekRuntime* runtime,
                           const void* initial_state, size_t state_size,
                           KekStateStorageCheckFn check) {
    if (!storage || !runtime || !initial_state || state_size == 0 || !check) {
        return 0;
    }

    memset(storage, 0, sizeof(*storage));
    storage->buffers[0] = (unsigned char*)malloc(state_size);
    storage->buffers[1] = (unsigned char*)malloc(state_size);
    if (!storage->buffers[0] || !storage->buffers[1]) {
        kek_state_storage_destroy(storage);
        return 0;
    }

    memcpy(storage->buffers[0], initial_state, state_size);
    memcpy(storage->buffers[1], initial_state, state_size);
    if (!check(storage->buffers[0])) {
        kek_state_storage_destroy(storage);
        return 0;
    }

    storage->runtime = runtime;
    storage->state_size = state_size;
    storage->active_index = 0;
    storage->check = check;
    return 1;
}

void kek_state_storage_destroy(KekStateStorage* storage) {
    if (!storage) {
        return;
    }

    free(storage->buffers[0]);
    free(storage->buffers[1]);
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
    memcpy(draft, current, storage->state_size);

    update(draft, context);
    if (!storage->check(draft)) {
        memcpy(draft, current, storage->state_size);
        return 0;
    }

    storage->active_index = inactive_index;
    if (kek_runtime_publish_state_changed(storage->runtime,
                                          kek_state_storage_current(storage))) {
        return 1;
    }

    storage->active_index = inactive_index == 0 ? 1u : 0u;
    memcpy(draft, current, storage->state_size);
    return 0;
}
