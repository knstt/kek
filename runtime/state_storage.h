#ifndef KEK_RUNTIME_STATE_STORAGE_H
#define KEK_RUNTIME_STATE_STORAGE_H

#include <stddef.h>
#include <stdint.h>

#include "runtime.h"

typedef int (*KekStateStorageCheckFn)(const void* state);
typedef void (*KekStateStorageUpdateFn)(void* draft, void* context);
struct KekHookDescriptor;

typedef struct KekStateStoreUpdateItem {
    size_t slot_id;
    KekStateStorageUpdateFn update;
    void* context;
    uint64_t changed_fields;
} KekStateStoreUpdateItem;

#define KEK_STATE_STORE_MAX_SLOTS 128
#define KEK_STATE_INVALID_ID ((size_t)-1)

typedef void (*KekStateDefaultIntoFn)(void* state);
typedef int (*KekStateResetFn)(void* state);

typedef struct KekStateDescriptor {
    size_t type_id;
    const char* name;
    size_t size;
    KekStateDefaultIntoFn set_default;
    KekStateStorageCheckFn check;
    KekStateResetFn reset;
} KekStateDescriptor;

typedef struct KekStateSlot {
    const KekStateDescriptor* descriptor;
    unsigned char* buffers[2];
    size_t active_index;
    uint64_t version;
    int in_use;
} KekStateSlot;

typedef struct KekStateStoreHookExecution {
    const struct KekHookDescriptor* descriptor;
    /* Informational only; write authorization is based on descriptor->writes. */
    size_t trigger_state_slot;
} KekStateStoreHookExecution;

typedef struct KekStateStore {
    KekRuntime* runtime;
    KekStateSlot slots[KEK_STATE_STORE_MAX_SLOTS];
    size_t slot_count;
    KekStateStoreHookExecution active_hook;
    struct KekStateStoreTransaction* active_transaction;
} KekStateStore;

typedef struct KekStateStoreTransactionSlot {
    const KekStateDescriptor* descriptor;
    unsigned char* buffers[2];
    size_t active_index;
    uint64_t version;
    uint64_t pending_version;
    int in_use;
    int recorded;
    int dirty;
    int created;
    int deleted;
    int owns_buffers;
    size_t draft_index;
} KekStateStoreTransactionSlot;

typedef struct KekStateStoreTransaction {
    KekStateStore* store;
    struct KekStateStoreTransaction* parent;
    KekStateStoreTransactionSlot slots[KEK_STATE_STORE_MAX_SLOTS];
    size_t slot_count;
} KekStateStoreTransaction;

typedef struct KekStateStorage {
    KekRuntime* runtime;
    unsigned char* buffers[2];
    size_t state_size;
    size_t active_index;
    KekStateStorageCheckFn check;
} KekStateStorage;

int kek_state_storage_init(KekStateStorage* storage, KekRuntime* runtime,
                           const void* initial_state, size_t state_size,
                           KekStateStorageCheckFn check);
void kek_state_storage_destroy(KekStateStorage* storage);
void* kek_state_storage_current(KekStateStorage* storage);
const void* kek_state_storage_current_const(const KekStateStorage* storage);
void* kek_state_storage_copy(KekStateStorage* storage, size_t index);
const void* kek_state_storage_copy_const(const KekStateStorage* storage, size_t index);
int kek_state_storage_update(KekStateStorage* storage,
                              KekStateStorageUpdateFn update, void* context);

void kek_state_store_init(KekStateStore* store, KekRuntime* runtime);
void kek_state_store_destroy(KekStateStore* store);
size_t kek_state_store_add(KekStateStore* store,
                           const KekStateDescriptor* descriptor,
                           const void* initial_state);
size_t kek_state_store_add_default(KekStateStore* store,
                                   const KekStateDescriptor* descriptor);
int kek_state_store_remove(KekStateStore* store, size_t slot_id);
void* kek_state_store_current(KekStateStore* store, size_t slot_id);
const void* kek_state_store_current_const(const KekStateStore* store, size_t slot_id);
const KekStateDescriptor* kek_state_store_descriptor(const KekStateStore* store,
                                                      size_t slot_id);
uint64_t kek_state_store_version(const KekStateStore* store, size_t slot_id);
size_t kek_state_store_find_first(const KekStateStore* store, size_t state_type_id);
size_t kek_state_store_find_next(const KekStateStore* store, size_t state_type_id,
                                 size_t after_slot_id);
int kek_state_store_update(KekStateStore* store, size_t slot_id,
                            KekStateStorageUpdateFn update, void* context);
int kek_state_store_update_fields(KekStateStore* store, size_t slot_id,
                                  KekStateStorageUpdateFn update, void* context,
                                  uint64_t changed_fields);
int kek_state_store_update_many(KekStateStore* store,
                                const KekStateStoreUpdateItem* updates,
                                size_t update_count);
void kek_state_store_begin_hook(KekStateStore* store,
                                const struct KekHookDescriptor* descriptor,
                                size_t trigger_state_slot,
                                KekStateStoreHookExecution* previous);
void kek_state_store_end_hook(KekStateStore* store,
                              const KekStateStoreHookExecution* previous);
int kek_state_store_transaction_begin(KekStateStore* store,
                                      KekStateStoreTransaction* transaction);
int kek_state_store_transaction_begin_for_hook(
    KekStateStore* store, KekStateStoreTransaction* transaction,
    const struct KekHookDescriptor* descriptor);
void kek_state_store_transaction_commit(KekStateStoreTransaction* transaction);
void kek_state_store_transaction_rollback(KekStateStoreTransaction* transaction);

#endif
