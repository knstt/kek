#include "state_storage.h"

#include <stdlib.h>
#include <string.h>

#include "hook.h"
#include "trace.h"

static uint64_t state_store_trace_start(KekStateStore* store) {
    return kek_trace_enabled(store ? store->runtime : NULL) ? kek_trace_now_ns() : 0;
}

static void state_store_trace_end(KekStateStore* store, const char* name,
                                  uint64_t start) {
    if (kek_trace_enabled(store ? store->runtime : NULL)) {
        kek_trace_record_runtime(store->runtime, name, kek_trace_now_ns() - start);
    }
}

static void state_store_trace_copy(KekStateStore* store, const char* name,
                                   void* target, const void* source, size_t size) {
    uint64_t start = state_store_trace_start(store);
    memcpy(target, source, size);
    state_store_trace_end(store, name, start);
}

static void state_store_trace_update(KekStateStore* store,
                                     KekStateStorageUpdateFn update,
                                     void* draft, void* context) {
    uint64_t start = state_store_trace_start(store);
    update(draft, context);
    state_store_trace_end(store, "state_store_update_callback", start);
}

static int state_store_trace_check(KekStateStore* store,
                                   KekStateStorageCheckFn check,
                                   const void* state) {
    uint64_t start = state_store_trace_start(store);
    int ok = check(state);
    state_store_trace_end(store, "state_store_validation", start);
    return ok;
}

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

static void state_slot_clear(KekStateStore* store, KekStateSlot* slot) {
    if (!slot) {
        return;
    }
    size_t size = slot->descriptor ? slot->descriptor->size : 0;
    kek_trace_free(store ? store->runtime : NULL, slot->buffers[0], size);
    kek_trace_free(store ? store->runtime : NULL, slot->buffers[1], size);
    memset(slot, 0, sizeof(*slot));
}

static int state_store_publish_slot_event(KekStateStore* store, KekStateSlot* slot,
                                          size_t slot_id, KekEventType event_type,
                                          uint64_t changed_fields) {
    if (!store || !store->runtime || !slot || !slot->descriptor) {
        return 0;
    }

    const void* source = slot->buffers[slot->active_index];
    uint64_t version = slot->version;
    KekEvent event;
    memset(&event, 0, sizeof(event));
    event.type = event_type;
    event.source = (void*)source;
    event.state_type_id = slot->descriptor->type_id;
    event.state_slot_id = slot_id;
    event.state_version = version;
    event.changed_fields = changed_fields;
    if (slot->descriptor->size <= KEK_EVENT_STATE_SNAPSHOT_CAPACITY) {
        memcpy(event.state_snapshot.data, source, slot->descriptor->size);
        event.state_snapshot_size = slot->descriptor->size;
        event.has_state_snapshot = 1;
    }
    return kek_event_publish(kek_runtime_events(store->runtime), &event);
}

static int state_store_publish_slot_event_from(KekStateStore* store,
                                               const KekStateDescriptor* descriptor,
                                               const void* source,
                                               size_t slot_id,
                                               KekEventType event_type,
                                               uint64_t version,
                                               uint64_t changed_fields) {
    if (!store || !store->runtime || !descriptor || !source) {
        return 0;
    }

    KekEvent event;
    memset(&event, 0, sizeof(event));
    event.type = event_type;
    event.source = (void*)source;
    event.state_type_id = descriptor->type_id;
    event.state_slot_id = slot_id;
    event.state_version = version;
    event.changed_fields = changed_fields;
    if (descriptor->size <= KEK_EVENT_STATE_SNAPSHOT_CAPACITY) {
        memcpy(event.state_snapshot.data, source, descriptor->size);
        event.state_snapshot_size = descriptor->size;
        event.has_state_snapshot = 1;
    }
    return kek_event_publish(kek_runtime_events(store->runtime), &event);
}

static KekStateSlot* state_store_slot(KekStateStore* store, size_t slot_id);

static KekStateStoreTransactionSlot* state_store_transaction_slot(
    KekStateStoreTransaction* transaction, size_t slot_id) {
    if (!transaction || slot_id >= KEK_STATE_STORE_MAX_SLOTS) {
        return NULL;
    }
    return &transaction->slots[slot_id];
}

static KekStateStoreTransactionSlot* state_store_find_transaction_slot(
    KekStateStoreTransaction* transaction, size_t slot_id) {
    for (KekStateStoreTransaction* current = transaction; current;
         current = current->parent) {
        KekStateStoreTransactionSlot* entry =
            state_store_transaction_slot(current, slot_id);
        if (entry && entry->recorded) {
            return entry;
        }
    }
    return NULL;
}

static int state_store_transaction_record_slot(KekStateStoreTransaction* transaction,
                                               size_t slot_id) {
    if (!transaction || !transaction->store ||
        slot_id >= KEK_STATE_STORE_MAX_SLOTS) {
        return 0;
    }
    KekStateStoreTransactionSlot* entry =
        state_store_transaction_slot(transaction, slot_id);
    if (!entry || entry->recorded) {
        return entry != NULL;
    }

    KekStateSlot* slot = &transaction->store->slots[slot_id];
    entry->descriptor = slot->descriptor;
    entry->active_index = slot->active_index;
    entry->version = slot->version;
    entry->pending_version = slot->version;
    entry->in_use = slot->in_use;
    entry->recorded = 1;
    entry->draft_index = KEK_STATE_INVALID_ID;
    return 1;
}

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
        state_slot_clear(store, &store->slots[i]);
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

    KekStateStoreTransaction* transaction = store->active_transaction;
    KekStateStoreTransactionSlot* transaction_entry = NULL;
    if (transaction) {
        if (!state_store_transaction_record_slot(transaction, slot_id)) {
            return KEK_STATE_INVALID_ID;
        }
        transaction_entry = state_store_transaction_slot(transaction, slot_id);
        if (transaction_entry && transaction_entry->in_use &&
            transaction_entry->deleted && !transaction_entry->owns_buffers) {
            transaction_entry->buffers[0] = store->slots[slot_id].buffers[0];
            transaction_entry->buffers[1] = store->slots[slot_id].buffers[1];
            transaction_entry->owns_buffers = 1;
            store->slots[slot_id].buffers[0] = NULL;
            store->slots[slot_id].buffers[1] = NULL;
        }
    }

    KekStateSlot* slot = &store->slots[slot_id];
    memset(slot, 0, sizeof(*slot));
    slot->buffers[0] =
        (unsigned char*)kek_trace_malloc(store->runtime, descriptor->size);
    slot->buffers[1] =
        (unsigned char*)kek_trace_malloc(store->runtime, descriptor->size);
    if (!slot->buffers[0] || !slot->buffers[1]) {
        state_slot_clear(store, slot);
        return KEK_STATE_INVALID_ID;
    }

    if (initial_state) {
        state_store_trace_copy(store, "state_store_init_copy", slot->buffers[0],
                               initial_state, descriptor->size);
    } else if (descriptor->set_default) {
        descriptor->set_default(slot->buffers[0]);
    } else {
        memset(slot->buffers[0], 0, descriptor->size);
    }

    state_store_trace_copy(store, "state_store_init_copy", slot->buffers[1],
                           slot->buffers[0], descriptor->size);
    if (!state_store_trace_check(store, descriptor->check, slot->buffers[0])) {
        state_slot_clear(store, slot);
        return KEK_STATE_INVALID_ID;
    }

    slot->descriptor = descriptor;
    slot->active_index = 0;
    slot->version = 1;
    slot->in_use = 1;
    if (transaction_entry) {
        transaction_entry->created = 1;
        transaction_entry->deleted = 0;
        transaction_entry->dirty = 0;
        transaction_entry->pending_version = slot->version;
        transaction_entry->draft_index = slot->active_index;
    }
    if (!state_store_publish_slot_event(store, slot, slot_id,
                                        KEK_EVENT_STATE_CREATED,
                                        KEK_EVENT_CHANGED_FIELDS_NONE)) {
        state_slot_clear(store, slot);
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

    KekStateStoreTransaction* transaction = store->active_transaction;
    KekStateStoreTransactionSlot* entry =
        state_store_find_transaction_slot(transaction, slot_id);
    const void* source = entry && entry->dirty
                             ? slot->buffers[entry->draft_index]
                             : slot->buffers[slot->active_index];
    uint64_t version = entry && entry->dirty ? entry->pending_version : slot->version;
    if (!state_store_publish_slot_event_from(store, slot->descriptor, source, slot_id,
                                             KEK_EVENT_STATE_DELETED, version,
                                             KEK_EVENT_CHANGED_FIELDS_NONE)) {
        return 0;
    }
    if (transaction) {
        if (!state_store_transaction_record_slot(transaction, slot_id)) {
            return 0;
        }
        entry = state_store_transaction_slot(transaction, slot_id);
        if (entry->created && !entry->in_use) {
            state_slot_clear(store, slot);
        } else {
            entry->deleted = 1;
        }
        slot->in_use = 0;
        while (store->slot_count > transaction->slot_count &&
               !store->slots[store->slot_count - 1].in_use) {
            store->slot_count--;
        }
        return 1;
    }
    state_slot_clear(store, slot);
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

    KekStateStoreTransaction* transaction = store->active_transaction;
    if (transaction) {
        if (!state_store_transaction_record_slot(transaction, slot_id)) {
            return 0;
        }
        KekStateStoreTransactionSlot* entry =
            state_store_transaction_slot(transaction, slot_id);
        KekStateStoreTransactionSlot* visible_entry =
            state_store_find_transaction_slot(transaction->parent, slot_id);
        int inherited_dirty = visible_entry && visible_entry->dirty;
        size_t draft_index;
        void* draft;
        const void* rollback_source = NULL;
        unsigned char* rollback_copy = NULL;

        if (!entry->dirty) {
            if (inherited_dirty) {
                draft_index = visible_entry->draft_index;
                draft = slot->buffers[draft_index];
                rollback_copy =
                    (unsigned char*)kek_trace_malloc(store->runtime,
                                                     slot->descriptor->size);
                if (!rollback_copy) {
                    return 0;
                }
                state_store_trace_copy(store, "state_store_transaction_copy",
                                       rollback_copy, draft,
                                       slot->descriptor->size);
                entry->buffers[0] = rollback_copy;
                entry->owns_buffers = 1;
                entry->pending_version = visible_entry->pending_version;
            } else {
                draft_index = slot->active_index == 0 ? 1u : 0u;
                draft = slot->buffers[draft_index];
                rollback_source = slot->buffers[slot->active_index];
                state_store_trace_copy(store, "state_store_draft_copy", draft,
                                       rollback_source,
                                       slot->descriptor->size);
            }
            entry->dirty = 1;
            entry->draft_index = draft_index;
        } else {
            draft_index = entry->draft_index;
            draft = slot->buffers[draft_index];
            rollback_copy =
                (unsigned char*)kek_trace_malloc(store->runtime,
                                                 slot->descriptor->size);
            if (!rollback_copy) {
                return 0;
            }
            state_store_trace_copy(store, "state_store_transaction_copy",
                                   rollback_copy, draft,
                                   slot->descriptor->size);
        }

        state_store_trace_update(store, update, draft, context);
        if (!state_store_trace_check(store, slot->descriptor->check, draft)) {
            if (rollback_copy) {
                state_store_trace_copy(store, "state_store_rollback_copy", draft,
                                       rollback_copy, slot->descriptor->size);
                kek_trace_free(store->runtime, rollback_copy,
                               slot->descriptor->size);
            } else if (rollback_source) {
                state_store_trace_copy(store, "state_store_rollback_copy", draft,
                                       rollback_source,
                                       slot->descriptor->size);
            }
            if (entry->owns_buffers && entry->buffers[0] == rollback_copy) {
                entry->buffers[0] = NULL;
                entry->owns_buffers = 0;
            }
            if (entry->pending_version == entry->version && !entry->created) {
                entry->dirty = 0;
                entry->draft_index = KEK_STATE_INVALID_ID;
            }
            return 0;
        }

        uint64_t previous_version = entry->pending_version;
        entry->pending_version++;
        if (state_store_publish_slot_event_from(
                store, slot->descriptor, draft, slot_id,
                KEK_EVENT_STATE_CHANGED, entry->pending_version,
                changed_fields)) {
            if (rollback_copy && !(entry->owns_buffers &&
                                   entry->buffers[0] == rollback_copy)) {
                kek_trace_free(store->runtime, rollback_copy,
                               slot->descriptor->size);
            }
            return 1;
        }

        entry->pending_version = previous_version;
        if (rollback_copy) {
            state_store_trace_copy(store, "state_store_rollback_copy", draft,
                                   rollback_copy, slot->descriptor->size);
            if (entry->owns_buffers && entry->buffers[0] == rollback_copy) {
                entry->buffers[0] = NULL;
                entry->owns_buffers = 0;
            }
            kek_trace_free(store->runtime, rollback_copy, slot->descriptor->size);
        } else if (rollback_source) {
            state_store_trace_copy(store, "state_store_rollback_copy", draft,
                                   rollback_source, slot->descriptor->size);
            entry->dirty = 0;
            entry->draft_index = KEK_STATE_INVALID_ID;
        }
        return 0;
    }

    size_t inactive_index = slot->active_index == 0 ? 1u : 0u;
    void* current = slot->buffers[slot->active_index];
    void* draft = slot->buffers[inactive_index];
    state_store_trace_copy(store, "state_store_draft_copy", draft, current,
                           slot->descriptor->size);

    state_store_trace_update(store, update, draft, context);
    if (!state_store_trace_check(store, slot->descriptor->check, draft)) {
        state_store_trace_copy(store, "state_store_rollback_copy", draft, current,
                               slot->descriptor->size);
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
    state_store_trace_copy(store, "state_store_rollback_copy", draft, current,
                           slot->descriptor->size);
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

    if (store->active_transaction) {
        KekStateStoreTransaction transaction;
        KekEventTransaction event_transaction;
        if (!kek_state_store_transaction_begin(store, &transaction) ||
            !kek_event_transaction_begin(kek_runtime_events(store->runtime),
                                         &event_transaction)) {
            kek_state_store_transaction_rollback(&transaction);
            return 0;
        }
        for (size_t i = 0; i < update_count; i++) {
            if (!kek_state_store_update_fields(store, updates[i].slot_id,
                                               updates[i].update,
                                               updates[i].context,
                                               updates[i].changed_fields)) {
                kek_event_transaction_rollback(&event_transaction);
                kek_state_store_transaction_rollback(&transaction);
                return 0;
            }
        }
        if (!state_store_publish_batch_event(store)) {
            kek_event_transaction_rollback(&event_transaction);
            kek_state_store_transaction_rollback(&transaction);
            return 0;
        }
        kek_event_transaction_commit(&event_transaction);
        kek_state_store_transaction_commit(&transaction);
        return 1;
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
        state_store_trace_copy(store, "state_store_draft_copy",
                               slot->buffers[inactive_indices[i]],
                               slot->buffers[slot->active_index],
                               slot->descriptor->size);
    }

    for (size_t i = 0; i < update_count; i++) {
        KekStateSlot* slot = slots[i];
        void* draft = slot->buffers[inactive_indices[i]];
        state_store_trace_update(store, updates[i].update, draft,
                                 updates[i].context);
        if (!state_store_trace_check(store, slot->descriptor->check, draft)) {
            for (size_t j = 0; j <= i; j++) {
                KekStateSlot* rollback_slot = slots[j];
                state_store_trace_copy(
                    store, "state_store_rollback_copy",
                    rollback_slot->buffers[inactive_indices[j]],
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
                state_store_trace_copy(store, "state_store_rollback_copy",
                                       slots[j]->buffers[inactive_indices[j]],
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
        state_store_trace_copy(store, "state_store_rollback_copy",
                               slots[i]->buffers[inactive_indices[i]],
                               slots[i]->buffers[active_indices[i]],
                               slots[i]->descriptor->size);
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
    for (size_t i = 0; i < KEK_STATE_STORE_MAX_SLOTS; i++) {
        KekStateStoreTransactionSlot* entry = &transaction->slots[i];
        size_t size = entry->descriptor ? entry->descriptor->size : 0;
        if (entry->owns_buffers) {
            kek_trace_free(transaction->store ? transaction->store->runtime : NULL,
                           entry->buffers[0], size);
            kek_trace_free(transaction->store ? transaction->store->runtime : NULL,
                           entry->buffers[1], size);
        }
        entry->buffers[0] = NULL;
        entry->buffers[1] = NULL;
    }
    memset(transaction, 0, sizeof(*transaction));
}

int kek_state_store_transaction_begin_for_hook(
    KekStateStore* store, KekStateStoreTransaction* transaction,
    const KekHookDescriptor* descriptor) {
    (void)descriptor;
    if (!store || !transaction) {
        return 0;
    }
    memset(transaction, 0, sizeof(*transaction));
    transaction->store = store;
    transaction->parent = store->active_transaction;
    transaction->slot_count = store->slot_count;
    store->active_transaction = transaction;
    return 1;
}

int kek_state_store_transaction_begin(KekStateStore* store,
                                      KekStateStoreTransaction* transaction) {
    return kek_state_store_transaction_begin_for_hook(store, transaction, NULL);
}

void kek_state_store_transaction_commit(KekStateStoreTransaction* transaction) {
    if (!transaction || !transaction->store) {
        return;
    }
    KekStateStore* store = transaction->store;
    if (store->active_transaction == transaction) {
        store->active_transaction = transaction->parent;
    }

    if (transaction->parent) {
        for (size_t i = 0; i < store->slot_count; i++) {
            KekStateStoreTransactionSlot* entry = &transaction->slots[i];
            if (!entry->recorded) {
                continue;
            }
            KekStateStoreTransactionSlot* parent_entry =
                state_store_transaction_slot(transaction->parent, i);
            if (!parent_entry) {
                continue;
            }
            if (!parent_entry->recorded) {
                parent_entry->descriptor = entry->descriptor;
                parent_entry->active_index = entry->active_index;
                parent_entry->version = entry->version;
                parent_entry->pending_version = entry->version;
                parent_entry->in_use = entry->in_use;
                parent_entry->recorded = 1;
                parent_entry->draft_index = KEK_STATE_INVALID_ID;
                if (entry->owns_buffers && entry->in_use &&
                    entry->buffers[0] && entry->buffers[1]) {
                    parent_entry->buffers[0] = entry->buffers[0];
                    parent_entry->buffers[1] = entry->buffers[1];
                    parent_entry->owns_buffers = 1;
                    entry->buffers[0] = NULL;
                    entry->buffers[1] = NULL;
                    entry->owns_buffers = 0;
                }
            }
            if (entry->dirty) {
                parent_entry->dirty = 1;
                parent_entry->draft_index = entry->draft_index;
                parent_entry->pending_version = entry->pending_version;
            }
            if (entry->created) {
                parent_entry->created = 1;
                parent_entry->deleted = 0;
                parent_entry->pending_version = entry->pending_version;
                parent_entry->draft_index = entry->draft_index;
            }
            if (entry->deleted) {
                parent_entry->deleted = 1;
            }
        }
        state_store_transaction_clear(transaction);
        return;
    }

    for (size_t i = 0; i < store->slot_count; i++) {
        KekStateStoreTransactionSlot* entry = &transaction->slots[i];
        if (!entry->recorded) {
            continue;
        }
        KekStateSlot* slot = &store->slots[i];
        if (entry->deleted && !entry->created) {
            state_slot_clear(store, slot);
            continue;
        }
        if (entry->dirty && slot->in_use) {
            slot->active_index = entry->draft_index;
            slot->version = entry->pending_version;
        }
    }
    while (store->slot_count > 0 && !store->slots[store->slot_count - 1].in_use) {
        store->slot_count--;
    }
    state_store_transaction_clear(transaction);
}

void kek_state_store_transaction_rollback(KekStateStoreTransaction* transaction) {
    if (!transaction || !transaction->store) {
        return;
    }
    KekStateStore* store = transaction->store;
    if (store->active_transaction == transaction) {
        store->active_transaction = transaction->parent;
    }
    for (size_t i = transaction->slot_count; i < store->slot_count; i++) {
        state_slot_clear(store, &store->slots[i]);
    }
    store->slot_count = transaction->slot_count;
    for (size_t i = 0; i < transaction->slot_count; i++) {
        KekStateStoreTransactionSlot* snapshot = &transaction->slots[i];
        if (!snapshot->recorded) {
            continue;
        }
        KekStateSlot* slot = &store->slots[i];
        if (snapshot->owns_buffers && snapshot->in_use &&
            snapshot->buffers[0] && snapshot->buffers[1]) {
            state_slot_clear(store, slot);
            memset(slot, 0, sizeof(*slot));
            slot->descriptor = snapshot->descriptor;
            slot->buffers[0] = snapshot->buffers[0];
            slot->buffers[1] = snapshot->buffers[1];
            slot->active_index = snapshot->active_index;
            slot->version = snapshot->version;
            slot->in_use = snapshot->in_use;
            snapshot->buffers[0] = NULL;
            snapshot->buffers[1] = NULL;
            snapshot->owns_buffers = 0;
        } else if (snapshot->owns_buffers && snapshot->dirty &&
                   snapshot->buffers[0]) {
            state_store_trace_copy(store, "state_store_rollback_copy",
                                   slot->buffers[snapshot->draft_index],
                                   snapshot->buffers[0],
                                   snapshot->descriptor->size);
        } else {
            if (snapshot->created && !snapshot->in_use) {
                state_slot_clear(store, slot);
                memset(slot, 0, sizeof(*slot));
                continue;
            }
            slot->descriptor = snapshot->descriptor;
            slot->active_index = snapshot->active_index;
            slot->version = snapshot->version;
            slot->in_use = snapshot->in_use;
        }
    }
    state_store_transaction_clear(transaction);
}
