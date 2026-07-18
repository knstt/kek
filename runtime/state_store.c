#include "state_storage.h"

#include <stdlib.h>
#include <string.h>

#include "hook.h"

static int state_store_type_write_allowed(const KekStateStore* store,
                                          size_t state_type_id) {
    const KekHookDescriptor* hook = store ? store->active_hook.descriptor : NULL;
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

static int state_store_write_allowed(const KekStateStore* store,
                                     const KekStateSlot* slot) {
    if (!slot || !slot->descriptor) {
        return 1;
    }
    return state_store_type_write_allowed(store, slot->descriptor->type_id);
}

void kek_state_store_init(KekStateStore* store, KekRuntime* runtime) {
    if (!store) {
        return;
    }
    memset(store, 0, sizeof(*store));
    store->runtime = runtime;
    store->active_hook.trigger_state_slot = KEK_STATE_INVALID_ID;
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
                                          size_t slot_id, KekEventType event_type,
                                          uint64_t changed_fields) {
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
    event.changed_fields = changed_fields;
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
    if (!state_store_type_write_allowed(store, descriptor->type_id)) {
        return KEK_STATE_INVALID_ID;
    }
    if (kek_event_capacity_remaining(kek_runtime_events(store->runtime)) == 0) {
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
    if (!state_store_publish_slot_event(store, slot, slot_id,
                                        KEK_EVENT_STATE_CREATED,
                                        KEK_EVENT_CHANGED_FIELDS_NONE)) {
        state_slot_clear(slot);
        while (store->slot_count > 0 && !store->slots[store->slot_count - 1].in_use) {
            store->slot_count--;
        }
        return KEK_STATE_INVALID_ID;
    }
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
    if (!state_store_write_allowed(store, slot)) {
        return 0;
    }
    if (kek_event_capacity_remaining(kek_runtime_events(store->runtime)) == 0) {
        return 0;
    }

    if (!state_store_publish_slot_event(store, slot, slot_id,
                                        KEK_EVENT_STATE_DELETED,
                                        KEK_EVENT_CHANGED_FIELDS_NONE)) {
        return 0;
    }
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

const void* kek_state_store_current_const(const KekStateStore* store,
                                          size_t slot_id) {
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
        if (slot->in_use && slot->descriptor &&
            slot->descriptor->type_id == state_type_id) {
            return i;
        }
    }
    return KEK_STATE_INVALID_ID;
}

int kek_state_store_update(KekStateStore* store, size_t slot_id,
                           KekStateStorageUpdateFn update, void* context) {
    return kek_state_store_update_fields(store, slot_id, update, context,
                                         KEK_EVENT_CHANGED_FIELDS_UNKNOWN);
}

int kek_state_store_update_fields(KekStateStore* store, size_t slot_id,
                                  KekStateStorageUpdateFn update, void* context,
                                  uint64_t changed_fields) {
    KekStateSlot* slot = state_store_slot(store, slot_id);
    if (!store || !store->runtime || !slot || !slot->descriptor ||
        !slot->descriptor->check || !update) {
        return 0;
    }
    if (!state_store_write_allowed(store, slot)) {
        return 0;
    }
    if (kek_event_capacity_remaining(kek_runtime_events(store->runtime)) == 0) {
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

    if (state_store_publish_slot_event(store, slot, slot_id,
                                       KEK_EVENT_STATE_CHANGED,
                                       changed_fields)) {
        return 1;
    }

    slot->active_index = inactive_index == 0 ? 1u : 0u;
    slot->version--;
    memcpy(draft, current, slot->descriptor->size);
    return 0;
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
    if (kek_event_capacity_remaining(kek_runtime_events(store->runtime)) <
        update_count + 1) {
        return 0;
    }

    KekStateSlot* slots[KEK_STATE_STORE_MAX_SLOTS];
    size_t inactive_indices[KEK_STATE_STORE_MAX_SLOTS];
    size_t active_indices[KEK_STATE_STORE_MAX_SLOTS];
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
        if (!state_store_write_allowed(store, slot)) {
            return 0;
        }
        slots[i] = slot;
        active_indices[i] = slot->active_index;
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
        if (!state_store_publish_slot_event(store, slots[i], updates[i].slot_id,
                                            KEK_EVENT_STATE_CHANGED,
                                            updates[i].changed_fields)) {
            for (size_t j = 0; j < update_count; j++) {
                slots[j]->active_index = active_indices[j];
                slots[j]->version--;
                memcpy(slots[j]->buffers[inactive_indices[j]],
                       slots[j]->buffers[active_indices[j]],
                       slots[j]->descriptor->size);
            }
            return 0;
        }
    }
    if (state_store_publish_batch_event(store)) {
        return 1;
    }
    for (size_t i = 0; i < update_count; i++) {
        slots[i]->active_index = active_indices[i];
        slots[i]->version--;
        memcpy(slots[i]->buffers[inactive_indices[i]],
               slots[i]->buffers[active_indices[i]], slots[i]->descriptor->size);
    }
    return 0;
}

void kek_state_store_begin_hook(KekStateStore* store,
                                const struct KekHookDescriptor* descriptor,
                                size_t trigger_state_slot,
                                KekStateStoreHookExecution* previous) {
    if (!store) {
        return;
    }
    if (previous) {
        *previous = store->active_hook;
    }
    store->active_hook.descriptor = descriptor;
    store->active_hook.trigger_state_slot = trigger_state_slot;
}

void kek_state_store_end_hook(KekStateStore* store,
                              const KekStateStoreHookExecution* previous) {
    if (!store) {
        return;
    }
    if (previous) {
        store->active_hook = *previous;
        return;
    }
    store->active_hook.descriptor = NULL;
    store->active_hook.trigger_state_slot = KEK_STATE_INVALID_ID;
}

static void state_store_transaction_clear(KekStateStoreTransaction* transaction) {
    if (!transaction) {
        return;
    }
    for (size_t i = 0; i < transaction->slot_count; i++) {
        free(transaction->slots[i].buffers[0]);
        free(transaction->slots[i].buffers[1]);
        transaction->slots[i].buffers[0] = NULL;
        transaction->slots[i].buffers[1] = NULL;
    }
    memset(transaction, 0, sizeof(*transaction));
}

int kek_state_store_transaction_begin(KekStateStore* store,
                                      KekStateStoreTransaction* transaction) {
    if (!store || !transaction) {
        return 0;
    }
    memset(transaction, 0, sizeof(*transaction));
    transaction->store = store;
    transaction->slot_count = store->slot_count;

    for (size_t i = 0; i < store->slot_count; i++) {
        KekStateSlot* slot = &store->slots[i];
        KekStateStoreTransactionSlot* snapshot = &transaction->slots[i];
        snapshot->descriptor = slot->descriptor;
        snapshot->active_index = slot->active_index;
        snapshot->version = slot->version;
        snapshot->in_use = slot->in_use;
        if (!slot->in_use || !slot->descriptor) {
            continue;
        }
        snapshot->buffers[0] = (unsigned char*)malloc(slot->descriptor->size);
        snapshot->buffers[1] = (unsigned char*)malloc(slot->descriptor->size);
        if (!snapshot->buffers[0] || !snapshot->buffers[1]) {
            state_store_transaction_clear(transaction);
            return 0;
        }
        memcpy(snapshot->buffers[0], slot->buffers[0], slot->descriptor->size);
        memcpy(snapshot->buffers[1], slot->buffers[1], slot->descriptor->size);
    }
    return 1;
}

void kek_state_store_transaction_commit(KekStateStoreTransaction* transaction) {
    state_store_transaction_clear(transaction);
}

void kek_state_store_transaction_rollback(KekStateStoreTransaction* transaction) {
    if (!transaction || !transaction->store) {
        return;
    }
    KekStateStore* store = transaction->store;
    for (size_t i = 0; i < store->slot_count; i++) {
        state_slot_clear(&store->slots[i]);
    }
    store->slot_count = transaction->slot_count;
    for (size_t i = 0; i < KEK_STATE_STORE_MAX_SLOTS; i++) {
        KekStateStoreTransactionSlot* snapshot = &transaction->slots[i];
        KekStateSlot* slot = &store->slots[i];
        memset(slot, 0, sizeof(*slot));
        if (i >= transaction->slot_count || !snapshot->in_use) {
            continue;
        }
        slot->descriptor = snapshot->descriptor;
        slot->buffers[0] = snapshot->buffers[0];
        slot->buffers[1] = snapshot->buffers[1];
        slot->active_index = snapshot->active_index;
        slot->version = snapshot->version;
        slot->in_use = snapshot->in_use;
        snapshot->buffers[0] = NULL;
        snapshot->buffers[1] = NULL;
    }
    state_store_transaction_clear(transaction);
}
