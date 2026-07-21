#ifndef KEK_RUNTIME_STATE_STORE_H
#define KEK_RUNTIME_STATE_STORE_H

#include <stddef.h>
#include <stdint.h>

#include "runtime.h"

typedef int (*KekStateCheckFn)(const void* state);
typedef void (*KekStateUpdateFn)(void* draft, void* context);
struct KekHookDescriptor;

typedef size_t KekStateHandle;

typedef struct KekStateStoreUpdateItem {
    KekStateHandle handle;
    KekStateUpdateFn update;
    void* context;
    uint64_t changed_fields;
} KekStateStoreUpdateItem;

#define KEK_STATE_STORE_MAX_SLOTS 128
#define KEK_STATE_INVALID_ID ((size_t)-1)

#define KEK_STATE_HANDLE_INDEX_BITS 16u
#define KEK_STATE_HANDLE_GENERATION_BITS 16u
#define KEK_STATE_HANDLE_TYPE_BITS 31u
#define KEK_STATE_HANDLE_INDEX_MASK ((size_t)((1ull << KEK_STATE_HANDLE_INDEX_BITS) - 1ull))
#define KEK_STATE_HANDLE_GENERATION_MASK ((size_t)((1ull << KEK_STATE_HANDLE_GENERATION_BITS) - 1ull))
#define KEK_STATE_HANDLE_TYPE_MASK ((size_t)((1ull << KEK_STATE_HANDLE_TYPE_BITS) - 1ull))
#define KEK_STATE_HANDLE_GENERATION_SHIFT KEK_STATE_HANDLE_INDEX_BITS
#define KEK_STATE_HANDLE_TYPE_SHIFT (KEK_STATE_HANDLE_INDEX_BITS + KEK_STATE_HANDLE_GENERATION_BITS)

#define KEK_STATE_ARENA_BLOCK_CAPACITY 4096u
#define KEK_STATE_ARENA_EMBEDDED_CAPACITY 8192u

typedef struct KekStateArenaBlock {
    unsigned char* data;
    size_t capacity;
    size_t used;
    int embedded;
    struct KekStateArenaBlock* next;
} KekStateArenaBlock;

typedef struct KekStateArena {
    KekRuntime* runtime;
    KekStateArenaBlock embedded_block;
    unsigned char embedded_data[KEK_STATE_ARENA_EMBEDDED_CAPACITY];
    KekStateArenaBlock* first;
    KekStateArenaBlock* current;
    size_t high_water_mark;
    size_t current_bytes;
} KekStateArena;

typedef void (*KekStateDefaultIntoFn)(void* state);
typedef int (*KekStateResetFn)(void* state);
typedef int (*KekStateMergeFieldsFn)(void* target, const void* source,
                                     uint64_t fields);

typedef struct KekStateFieldDescriptor {
    const char* name;
    uint64_t mask;
    size_t offset;
    size_t size;
    size_t alignment;
    int is_blob;
} KekStateFieldDescriptor;

typedef struct KekStateDescriptor {
    size_t type_id;
    const char* name;
    size_t size;
    size_t alignment;
    size_t pool_capacity;
    KekStateDefaultIntoFn set_default;
    KekStateCheckFn check;
    KekStateResetFn reset;
    KekStateMergeFieldsFn merge_fields;
    const KekStateFieldDescriptor* fields;
    size_t field_count;
} KekStateDescriptor;

typedef struct KekStateSlot {
    const KekStateDescriptor* descriptor;
    unsigned char* buffers[2];
    unsigned char buffer_owned[2];
    size_t active_index;
    size_t pool_index;
    uint64_t version;
    uint32_t generation;
    int in_use;
} KekStateSlot;

typedef struct KekStateTypePool {
    size_t state_type_id;
    const KekStateDescriptor* descriptor;
    unsigned char* records[2];
    unsigned char record_owned[2];
    size_t capacity;
    KekStateHandle handles[KEK_STATE_STORE_MAX_SLOTS];
    size_t slot_indices[KEK_STATE_STORE_MAX_SLOTS];
    size_t count;
    int in_use;
} KekStateTypePool;

typedef struct KekStateStoreHookExecution {
    const struct KekHookDescriptor* descriptor;
    /* Informational only; write authorization is based on descriptor->writes. */
    size_t trigger_state_slot;
} KekStateStoreHookExecution;

typedef struct KekStateStore {
    KekRuntime* runtime;
    KekStateSlot slots[KEK_STATE_STORE_MAX_SLOTS];
    KekStateTypePool type_pools[KEK_STATE_STORE_MAX_SLOTS];
    size_t slot_count;
    KekStateArena arena;
    KekStateArena* allocator;
    int owns_allocator;
    int owns_type_pool_records;
    KekStateStoreHookExecution active_hook;
    struct KekStateStoreTransaction* active_transaction;
} KekStateStore;

typedef struct KekStateStoreTransactionSlot {
    const KekStateDescriptor* descriptor;
    unsigned char* buffers[2];
    unsigned char buffer_owned[2];
    size_t active_index;
    size_t pool_index;
    uint64_t version;
    uint64_t pending_version;
    uint32_t generation;
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

void kek_state_arena_init(KekStateArena* arena, KekRuntime* runtime);
void kek_state_arena_destroy(KekStateArena* arena);
void kek_state_arena_reset(KekStateArena* arena);
void* kek_state_arena_alloc(KekStateArena* arena, size_t size, size_t alignment);
size_t kek_state_arena_high_water_mark(const KekStateArena* arena);

KekStateHandle kek_state_handle_make(size_t state_type_id, size_t index,
                                     uint32_t generation);
size_t kek_state_handle_index(KekStateHandle handle);
size_t kek_state_handle_type_id(KekStateHandle handle);
uint32_t kek_state_handle_generation(KekStateHandle handle);

void kek_state_store_init(KekStateStore* store, KekRuntime* runtime);
void kek_state_store_init_with_allocator(KekStateStore* store, KekRuntime* runtime,
                                         KekStateArena* allocator);
void kek_state_store_destroy(KekStateStore* store);
KekStateHandle kek_state_store_add(KekStateStore* store,
                                   const KekStateDescriptor* descriptor,
                                   const void* initial_state);
KekStateHandle kek_state_store_add_default(KekStateStore* store,
                                           const KekStateDescriptor* descriptor);
int kek_state_store_remove(KekStateStore* store, KekStateHandle handle);
void* kek_state_store_current(KekStateStore* store, KekStateHandle handle);
const void* kek_state_store_current_const(const KekStateStore* store,
                                          KekStateHandle handle);
const KekStateDescriptor* kek_state_store_descriptor(const KekStateStore* store,
                                                     KekStateHandle handle);
uint64_t kek_state_store_version(const KekStateStore* store, KekStateHandle handle);
KekStateHandle kek_state_store_find_first(const KekStateStore* store,
                                          size_t state_type_id);
KekStateHandle kek_state_store_find_next(const KekStateStore* store,
                                         size_t state_type_id,
                                         KekStateHandle after_handle);
int kek_state_store_update(KekStateStore* store, KekStateHandle handle,
                            KekStateUpdateFn update, void* context);
int kek_state_store_update_fields(KekStateStore* store, KekStateHandle handle,
                                  KekStateUpdateFn update, void* context,
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
