#ifndef KEK_RUNTIME_STATE_STORAGE_H
#define KEK_RUNTIME_STATE_STORAGE_H

#include <stddef.h>
#include <stdint.h>

#include "runtime.h"

typedef int (*KekStateStorageCheckFn)(const void* state);
typedef void (*KekStateStorageUpdateFn)(void* draft, void* context);

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

typedef struct KekStateStore {
    KekRuntime* runtime;
    KekStateSlot slots[KEK_STATE_STORE_MAX_SLOTS];
    size_t slot_count;
} KekStateStore;

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

#endif
