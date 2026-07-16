#include "state_storage.h"

#include <stdlib.h>
#include <string.h>

#include "hook.h"

static int state_store_type_write_allowed(size_t state_type_id) {
    const KekHookDescriptor* hook = kek_hook_current_descriptor();
    if (!hook) {
        return 1;
    }
    for (size_t i = 0; i < hook->write_count; i++) {
        if (hook->writes[i] == state_type_id) {
            return 1;
        }
    }
    return 0;
}

static int state_store_write_allowed(const KekStateSlot* slot, size_t slot_id) {
    if (!slot || !slot->descriptor) {
        return 1;
    }
    if (slot_id == kek_hook_current_trigger_state_slot()) {
        return 0;
    }
    return state_store_type_write_allowed(slot->descriptor->type_id);
}

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
    kek_runtime_publish_state_changed(storage->runtime, kek_state_storage_current(storage));
    return 1;
}

void kek_state_store_init(KekStateStore* store, KekRuntime* runtime) {
    if (!store) {
        return;
    }
    memset(store, 0, sizeof(*store));
    store->runtime = runtime;
}

static void state_slot_clear(KekStateSlot* slot) {
    if (!slot) {
        return;
    }
    free(slot->buffers[0]);
    free(slot->buffers[1]);
    memset(slot, 0, sizeof(*slot));
}

static int state_store_publish_slot_event(KekStateStore* store, KekStateSlot* slot,
                                           size_t slot_id, KekEventType event_type) {
    if (!store || !store->runtime || !slot || !slot->descriptor) {
        return 0;
    }

    KekEvent event;
    memset(&event, 0, sizeof(event));
    event.type = event_type;
    event.source = slot->buffers[slot->active_index];
    event.state_type_id = slot->descriptor->type_id;
    event.state_slot_id = slot_id;
    event.state_version = slot->version;
    if (slot->descriptor->size <= KEK_EVENT_STATE_SNAPSHOT_CAPACITY) {
        memcpy(event.state_snapshot.data, slot->buffers[slot->active_index],
               slot->descriptor->size);
        event.state_snapshot_size = slot->descriptor->size;
        event.has_state_snapshot = 1;
    }
    return kek_event_publish(kek_runtime_events(store->runtime), &event);
}

static KekStateSlot* state_store_slot(KekStateStore* store, size_t slot_id);

static int state_store_publish_batch_event(KekStateStore* store) {
    if (!store || !store->runtime) {
        return 0;
    }

    KekEvent event;
    memset(&event, 0, sizeof(event));
    event.type = KEK_EVENT_STATE_BATCH_CHANGED;
    return kek_event_publish(kek_runtime_events(store->runtime), &event);
}

void kek_state_store_destroy(KekStateStore* store) {
    if (!store) {
        return;
    }

    for (size_t i = 0; i < store->slot_count; i++) {
        state_slot_clear(&store->slots[i]);
    }
    memset(store, 0, sizeof(*store));
}

size_t kek_state_store_add(KekStateStore* store,
                           const KekStateDescriptor* descriptor,
                           const void* initial_state) {
    if (!store || !store->runtime || !descriptor || descriptor->size == 0 ||
        !descriptor->check) {
        return KEK_STATE_INVALID_ID;
    }
    if (!state_store_type_write_allowed(descriptor->type_id)) {
        return KEK_STATE_INVALID_ID;
    }

    size_t slot_id = KEK_STATE_INVALID_ID;
    for (size_t i = 0; i < store->slot_count; i++) {
        if (!store->slots[i].in_use) {
            slot_id = i;
            break;
        }
    }
    if (slot_id == KEK_STATE_INVALID_ID) {
        if (store->slot_count >= KEK_STATE_STORE_MAX_SLOTS) {
            return KEK_STATE_INVALID_ID;
        }
        slot_id = store->slot_count++;
    }

    KekStateSlot* slot = &store->slots[slot_id];
    memset(slot, 0, sizeof(*slot));
    slot->buffers[0] = (unsigned char*)malloc(descriptor->size);
    slot->buffers[1] = (unsigned char*)malloc(descriptor->size);
    if (!slot->buffers[0] || !slot->buffers[1]) {
        state_slot_clear(slot);
        return KEK_STATE_INVALID_ID;
    }

    if (initial_state) {
        memcpy(slot->buffers[0], initial_state, descriptor->size);
    } else if (descriptor->set_default) {
        descriptor->set_default(slot->buffers[0]);
    } else {
        memset(slot->buffers[0], 0, descriptor->size);
    }

    memcpy(slot->buffers[1], slot->buffers[0], descriptor->size);
    if (!descriptor->check(slot->buffers[0])) {
        state_slot_clear(slot);
        return KEK_STATE_INVALID_ID;
    }

    slot->descriptor = descriptor;
    slot->active_index = 0;
    slot->version = 1;
    slot->in_use = 1;
    state_store_publish_slot_event(store, slot, slot_id, KEK_EVENT_STATE_CREATED);
    return slot_id;
}

size_t kek_state_store_add_default(KekStateStore* store,
                                   const KekStateDescriptor* descriptor) {
    return kek_state_store_add(store, descriptor, NULL);
}

int kek_state_store_remove(KekStateStore* store, size_t slot_id) {
    KekStateSlot* slot = state_store_slot(store, slot_id);
    if (!slot) {
        return 0;
    }
    if (!state_store_write_allowed(slot, slot_id)) {
        return 0;
    }

    state_store_publish_slot_event(store, slot, slot_id, KEK_EVENT_STATE_DELETED);
    state_slot_clear(slot);
    while (store->slot_count > 0 && !store->slots[store->slot_count - 1].in_use) {
        store->slot_count--;
    }
    return 1;
}

static KekStateSlot* state_store_slot(KekStateStore* store, size_t slot_id) {
    if (!store || slot_id >= store->slot_count || !store->slots[slot_id].in_use) {
        return NULL;
    }
    return &store->slots[slot_id];
}

static const KekStateSlot* state_store_slot_const(const KekStateStore* store,
                                                  size_t slot_id) {
    if (!store || slot_id >= store->slot_count || !store->slots[slot_id].in_use) {
        return NULL;
    }
    return &store->slots[slot_id];
}

void* kek_state_store_current(KekStateStore* store, size_t slot_id) {
    KekStateSlot* slot = state_store_slot(store, slot_id);
    return slot ? slot->buffers[slot->active_index] : NULL;
}

const void* kek_state_store_current_const(const KekStateStore* store, size_t slot_id) {
    const KekStateSlot* slot = state_store_slot_const(store, slot_id);
    return slot ? slot->buffers[slot->active_index] : NULL;
}

const KekStateDescriptor* kek_state_store_descriptor(const KekStateStore* store,
                                                     size_t slot_id) {
    const KekStateSlot* slot = state_store_slot_const(store, slot_id);
    return slot ? slot->descriptor : NULL;
}

uint64_t kek_state_store_version(const KekStateStore* store, size_t slot_id) {
    const KekStateSlot* slot = state_store_slot_const(store, slot_id);
    return slot ? slot->version : 0;
}

size_t kek_state_store_find_first(const KekStateStore* store, size_t state_type_id) {
    return kek_state_store_find_next(store, state_type_id, KEK_STATE_INVALID_ID);
}

size_t kek_state_store_find_next(const KekStateStore* store, size_t state_type_id,
                                 size_t after_slot_id) {
    if (!store) {
        return KEK_STATE_INVALID_ID;
    }

    size_t start = after_slot_id == KEK_STATE_INVALID_ID ? 0 : after_slot_id + 1;
    for (size_t i = start; i < store->slot_count; i++) {
        const KekStateSlot* slot = &store->slots[i];
        if (slot->in_use && slot->descriptor && slot->descriptor->type_id == state_type_id) {
            return i;
        }
    }
    return KEK_STATE_INVALID_ID;
}

int kek_state_store_update(KekStateStore* store, size_t slot_id,
                            KekStateStorageUpdateFn update, void* context) {
    KekStateSlot* slot = state_store_slot(store, slot_id);
    if (!store || !store->runtime || !slot || !slot->descriptor ||
        !slot->descriptor->check || !update) {
        return 0;
    }
    if (!state_store_write_allowed(slot, slot_id)) {
        return 0;
    }

    size_t inactive_index = slot->active_index == 0 ? 1u : 0u;
    void* current = slot->buffers[slot->active_index];
    void* draft = slot->buffers[inactive_index];
    memcpy(draft, current, slot->descriptor->size);

    update(draft, context);
    if (!slot->descriptor->check(draft)) {
        memcpy(draft, current, slot->descriptor->size);
        return 0;
    }

    slot->active_index = inactive_index;
    slot->version++;

    state_store_publish_slot_event(store, slot, slot_id, KEK_EVENT_STATE_CHANGED);
    return 1;
}

int kek_state_store_update_many(KekStateStore* store,
                                const KekStateStoreUpdateItem* updates,
                                size_t update_count) {
    if (!store || !store->runtime || (!updates && update_count > 0)) {
        return 0;
    }
    if (update_count == 0) {
        return 1;
    }

    KekStateSlot* slots[KEK_STATE_STORE_MAX_SLOTS];
    size_t inactive_indices[KEK_STATE_STORE_MAX_SLOTS];
    if (update_count > KEK_STATE_STORE_MAX_SLOTS) {
        return 0;
    }

    for (size_t i = 0; i < update_count; i++) {
        if (!updates[i].update) {
            return 0;
        }
        for (size_t j = 0; j < i; j++) {
            if (updates[i].slot_id == updates[j].slot_id) {
                return 0;
            }
        }
        KekStateSlot* slot = state_store_slot(store, updates[i].slot_id);
        if (!slot || !slot->descriptor || !slot->descriptor->check) {
            return 0;
        }
        if (!state_store_write_allowed(slot, updates[i].slot_id)) {
            return 0;
        }
        slots[i] = slot;
        inactive_indices[i] = slot->active_index == 0 ? 1u : 0u;
        memcpy(slot->buffers[inactive_indices[i]], slot->buffers[slot->active_index],
               slot->descriptor->size);
    }

    for (size_t i = 0; i < update_count; i++) {
        KekStateSlot* slot = slots[i];
        void* draft = slot->buffers[inactive_indices[i]];
        updates[i].update(draft, updates[i].context);
        if (!slot->descriptor->check(draft)) {
            for (size_t j = 0; j <= i; j++) {
                KekStateSlot* rollback_slot = slots[j];
                memcpy(rollback_slot->buffers[inactive_indices[j]],
                       rollback_slot->buffers[rollback_slot->active_index],
                       rollback_slot->descriptor->size);
            }
            return 0;
        }
    }

    for (size_t i = 0; i < update_count; i++) {
        slots[i]->active_index = inactive_indices[i];
        slots[i]->version++;
    }

    for (size_t i = 0; i < update_count; i++) {
        state_store_publish_slot_event(store, slots[i], updates[i].slot_id,
                                       KEK_EVENT_STATE_CHANGED);
    }
    state_store_publish_batch_event(store);
    return 1;
}
