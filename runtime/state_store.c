#include "state_store.h"

#include <stdlib.h>
#include <string.h>

#include "hook.h"
#include "trace.h"

static uint64_t state_store_trace_start(KekStateStore* store) {
    return kek_trace_enabled(store ? store->runtime : NULL) ? kek_trace_now_ns() : 0;
}

static void state_store_trace_end(KekStateStore* store,
                                  KekTraceRuntimeMetricId metric_id,
                                  uint64_t start) {
    if (kek_trace_enabled(store ? store->runtime : NULL)) {
        kek_trace_record_runtime_metric(store->runtime, metric_id,
                                        kek_trace_now_ns() - start);
    }
}

static void state_store_trace_copy(KekStateStore* store,
                                   KekTraceRuntimeMetricId metric_id,
                                   void* target, const void* source, size_t size) {
    uint64_t start = state_store_trace_start(store);
    memcpy(target, source, size);
    state_store_trace_end(store, metric_id, start);
}

static void state_store_trace_update(KekStateStore* store,
                                     KekStateUpdateFn update,
                                     void* draft, void* context) {
    uint64_t start = state_store_trace_start(store);
    update(draft, context);
    state_store_trace_end(store, KEK_TRACE_METRIC_STATE_STORE_UPDATE_CALLBACK,
                          start);
}

static int state_store_trace_check(KekStateStore* store,
                                   KekStateCheckFn check,
                                   const void* state) {
    uint64_t start = state_store_trace_start(store);
    int ok = check(state);
    state_store_trace_end(store, KEK_TRACE_METRIC_STATE_STORE_VALIDATION, start);
    return ok;
}

KekStateHandle kek_state_handle_make(size_t state_type_id, size_t index,
                                     uint32_t generation) {
    if (index > KEK_STATE_HANDLE_INDEX_MASK ||
        state_type_id > KEK_STATE_HANDLE_TYPE_MASK) {
        return KEK_STATE_INVALID_ID;
    }
    size_t gen = generation & KEK_STATE_HANDLE_GENERATION_MASK;
    if (gen == 0) {
        gen = 1;
    }
    return (state_type_id << KEK_STATE_HANDLE_TYPE_SHIFT) |
           (gen << KEK_STATE_HANDLE_GENERATION_SHIFT) | index;
}

size_t kek_state_handle_index(KekStateHandle handle) {
    return handle == KEK_STATE_INVALID_ID ? KEK_STATE_INVALID_ID
                                          : handle & KEK_STATE_HANDLE_INDEX_MASK;
}

size_t kek_state_handle_type_id(KekStateHandle handle) {
    return handle == KEK_STATE_INVALID_ID
               ? KEK_STATE_INVALID_ID
               : (handle >> KEK_STATE_HANDLE_TYPE_SHIFT) & KEK_STATE_HANDLE_TYPE_MASK;
}

uint32_t kek_state_handle_generation(KekStateHandle handle) {
    return handle == KEK_STATE_INVALID_ID
               ? 0
               : (uint32_t)((handle >> KEK_STATE_HANDLE_GENERATION_SHIFT) &
                            KEK_STATE_HANDLE_GENERATION_MASK);
}

static const KekStateTypePool* state_store_type_pool_const(
    const KekStateStore* store, size_t state_type_id);
static unsigned char* state_store_alloc_buffer(KekStateStore* store, size_t size,
                                               size_t alignment,
                                               unsigned char* owned);

static KekStateHandle state_store_slot_handle(const KekStateStore* store,
                                              size_t slot_id) {
    if (!store || slot_id >= store->slot_count) {
        return KEK_STATE_INVALID_ID;
    }
    const KekStateSlot* slot = &store->slots[slot_id];
    if (!slot->in_use || !slot->descriptor) {
        return KEK_STATE_INVALID_ID;
    }
    if (slot->pool_index == KEK_STATE_INVALID_ID) {
        return KEK_STATE_INVALID_ID;
    }
    return kek_state_handle_make(slot->descriptor->type_id, slot->pool_index,
                                 slot->generation);
}

static size_t state_store_handle_index_checked(const KekStateStore* store,
                                               KekStateHandle handle) {
    if (!store || handle == KEK_STATE_INVALID_ID) {
        return KEK_STATE_INVALID_ID;
    }
    size_t pool_index = kek_state_handle_index(handle);
    const KekStateTypePool* pool =
        state_store_type_pool_const(store, kek_state_handle_type_id(handle));
    if (!pool || pool_index >= pool->count ||
        pool->handles[pool_index] != handle ||
        pool->slot_indices[pool_index] == KEK_STATE_INVALID_ID ||
        pool->slot_indices[pool_index] >= store->slot_count) {
        return KEK_STATE_INVALID_ID;
    }
    size_t slot_index = pool->slot_indices[pool_index];
    const KekStateSlot* slot = &store->slots[slot_index];
    if (!slot->in_use || !slot->descriptor ||
        slot->pool_index != pool_index ||
        slot->generation != kek_state_handle_generation(handle) ||
        slot->descriptor->type_id != kek_state_handle_type_id(handle)) {
        return KEK_STATE_INVALID_ID;
    }
    return slot_index;
}

static void state_store_type_pool_init(KekStateTypePool* pool,
                                       size_t state_type_id) {
    if (!pool) {
        return;
    }
    memset(pool, 0, sizeof(*pool));
    pool->in_use = 1;
    pool->state_type_id = state_type_id;
    pool->capacity = 0;
    for (size_t i = 0; i < KEK_STATE_STORE_MAX_SLOTS; i++) {
        pool->handles[i] = KEK_STATE_INVALID_ID;
        pool->slot_indices[i] = KEK_STATE_INVALID_ID;
    }
}

static int state_store_type_pool_ensure_records(
    KekStateStore* store, KekStateTypePool* pool,
    const KekStateDescriptor* descriptor) {
    if (!store || !pool || !descriptor || descriptor->size == 0) {
        return 0;
    }
    if (pool->descriptor && pool->descriptor != descriptor) {
        return 0;
    }
    pool->descriptor = descriptor;
    pool->capacity = descriptor->pool_capacity > 0
                         ? descriptor->pool_capacity
                         : KEK_STATE_STORE_MAX_SLOTS;
    if (pool->capacity > KEK_STATE_STORE_MAX_SLOTS) {
        return 0;
    }
    if (pool->records[0] && pool->records[1]) {
        return 1;
    }
    if (descriptor->size > ((size_t)-1) / pool->capacity) {
        return 0;
    }
    size_t bytes = descriptor->size * pool->capacity;
    for (size_t i = 0; i < 2; i++) {
        if (pool->records[i]) {
            continue;
        }
        pool->records[i] =
            state_store_alloc_buffer(store, bytes, descriptor->alignment,
                                     &pool->record_owned[i]);
        if (!pool->records[i]) {
            return 0;
        }
    }
    return 1;
}

static unsigned char* state_store_type_pool_record(KekStateTypePool* pool,
                                                   size_t pool_index,
                                                   size_t buffer_index) {
    if (!pool || pool_index >= pool->capacity || buffer_index >= 2 ||
        !pool->records[buffer_index] || !pool->descriptor) {
        return NULL;
    }
    return pool->records[buffer_index] + pool_index * pool->descriptor->size;
}

static void state_store_type_pool_destroy(KekStateStore* store,
                                          KekStateTypePool* pool) {
    if (!pool) {
        return;
    }
    size_t bytes =
        pool->descriptor ? pool->descriptor->size * pool->capacity : 0;
    for (size_t i = 0; i < 2; i++) {
        if (pool->record_owned[i] && pool->records[i]) {
            kek_trace_free(store ? store->runtime : NULL, pool->records[i], bytes);
        }
    }
    memset(pool, 0, sizeof(*pool));
}

static KekStateTypePool* state_store_type_pool(KekStateStore* store,
                                               size_t state_type_id,
                                               int create) {
    if (!store) {
        return NULL;
    }
    for (size_t i = 0; i < KEK_STATE_STORE_MAX_SLOTS; i++) {
        KekStateTypePool* pool = &store->type_pools[i];
        if (pool->in_use && pool->state_type_id == state_type_id) {
            return pool;
        }
    }
    if (!create) {
        return NULL;
    }
    for (size_t i = 0; i < KEK_STATE_STORE_MAX_SLOTS; i++) {
        KekStateTypePool* pool = &store->type_pools[i];
        if (!pool->in_use) {
            state_store_type_pool_init(pool, state_type_id);
            return pool;
        }
    }
    return NULL;
}

static const KekStateTypePool* state_store_type_pool_const(
    const KekStateStore* store, size_t state_type_id) {
    if (!store) {
        return NULL;
    }
    for (size_t i = 0; i < KEK_STATE_STORE_MAX_SLOTS; i++) {
        const KekStateTypePool* pool = &store->type_pools[i];
        if (pool->in_use && pool->state_type_id == state_type_id) {
            return pool;
        }
    }
    return NULL;
}

static KekStateHandle state_store_type_pool_add(KekStateStore* store,
                                                const KekStateDescriptor* descriptor,
                                                size_t slot_index,
                                                uint32_t generation,
                                                size_t* out_pool_index) {
    if (!descriptor) {
        return KEK_STATE_INVALID_ID;
    }
    KekStateTypePool* pool = state_store_type_pool(store, descriptor->type_id, 1);
    if (!pool || !state_store_type_pool_ensure_records(store, pool, descriptor)) {
        return KEK_STATE_INVALID_ID;
    }
    size_t pool_index = KEK_STATE_INVALID_ID;
    for (size_t i = 0; i < pool->count; i++) {
        if (pool->handles[i] == KEK_STATE_INVALID_ID) {
            pool_index = i;
            break;
        }
    }
    if (pool_index == KEK_STATE_INVALID_ID) {
        if (pool->count >= pool->capacity) {
            return KEK_STATE_INVALID_ID;
        }
        pool_index = pool->count++;
    }
    KekStateHandle handle =
        kek_state_handle_make(descriptor->type_id, pool_index, generation);
    if (handle == KEK_STATE_INVALID_ID) {
        return KEK_STATE_INVALID_ID;
    }
    pool->handles[pool_index] = handle;
    pool->slot_indices[pool_index] = slot_index;
    if (out_pool_index) {
        *out_pool_index = pool_index;
    }
    return handle;
}

static void state_store_type_pool_remove(KekStateStore* store,
                                         size_t state_type_id,
                                         KekStateHandle handle) {
    KekStateTypePool* pool = state_store_type_pool(store, state_type_id, 0);
    if (!pool) {
        return;
    }
    size_t pool_index = kek_state_handle_index(handle);
    if (pool_index >= pool->count || pool->handles[pool_index] != handle) {
        return;
    }
    pool->handles[pool_index] = KEK_STATE_INVALID_ID;
    pool->slot_indices[pool_index] = KEK_STATE_INVALID_ID;
    while (pool->count > 0 &&
           pool->handles[pool->count - 1] == KEK_STATE_INVALID_ID) {
        pool->count--;
    }
}

static int state_store_type_write_allowed(const KekStateStore* store,
                                          size_t state_type_id) {
    const KekHookDescriptor* hook = store ? store->active_hook.descriptor : NULL;
    if (!hook) {
        return 1;
    }
    if (hook->access_count > 0 && hook->accesses) {
        for (size_t i = 0; i < hook->access_count; i++) {
            const KekHookAccess* access = &hook->accesses[i];
            if ((access->mode == KEK_HOOK_ACCESS_WRITE ||
                 access->mode == KEK_HOOK_ACCESS_CREATE ||
                 access->mode == KEK_HOOK_ACCESS_DELETE) &&
                access->state_type_id == state_type_id &&
                access->scope != KEK_HOOK_ACCESS_SCOPE_EXACT_SLOT) {
                return 1;
            }
        }
        return 0;
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
    const KekHookDescriptor* hook = store ? store->active_hook.descriptor : NULL;
    if (hook && hook->access_count > 0 && hook->accesses) {
        size_t slot_id = (size_t)(slot - store->slots);
        KekStateHandle handle = state_store_slot_handle(store, slot_id);
        for (size_t i = 0; i < hook->access_count; i++) {
            const KekHookAccess* access = &hook->accesses[i];
            if ((access->mode == KEK_HOOK_ACCESS_WRITE ||
                 access->mode == KEK_HOOK_ACCESS_DELETE) &&
                access->state_type_id == slot->descriptor->type_id &&
                (access->scope != KEK_HOOK_ACCESS_SCOPE_EXACT_SLOT ||
                 access->state_slot_id == handle)) {
                return 1;
            }
        }
        return 0;
    }
    return state_store_type_write_allowed(store, slot->descriptor->type_id);
}

static size_t state_store_align_up(size_t value, size_t alignment) {
    size_t align = alignment ? alignment : sizeof(max_align_t);
    size_t remainder = value % align;
    return remainder == 0 ? value : value + (align - remainder);
}

void kek_state_arena_init(KekStateArena* arena, KekRuntime* runtime) {
    if (!arena) {
        return;
    }
    memset(arena, 0, sizeof(*arena));
    arena->runtime = runtime;
    arena->embedded_block.data = arena->embedded_data;
    arena->embedded_block.capacity = sizeof(arena->embedded_data);
    arena->embedded_block.embedded = 1;
    arena->first = &arena->embedded_block;
    arena->current = &arena->embedded_block;
}

void kek_state_arena_destroy(KekStateArena* arena) {
    if (!arena) {
        return;
    }
    if (kek_trace_enabled(arena->runtime)) {
        kek_trace_record_runtime_metric(arena->runtime,
                                        KEK_TRACE_METRIC_STATE_ARENA_HIGH_WATER,
                                        arena->high_water_mark);
    }
    KekStateArenaBlock* block = arena->first;
    while (block) {
        KekStateArenaBlock* next = block->next;
        if (!block->embedded) {
            kek_trace_free(arena->runtime, block->data, block->capacity);
            kek_trace_free(arena->runtime, block, sizeof(*block));
        }
        block = next;
    }
    memset(arena, 0, sizeof(*arena));
}

void kek_state_arena_reset(KekStateArena* arena) {
    if (!arena) {
        return;
    }
    if (kek_trace_enabled(arena->runtime)) {
        kek_trace_record_runtime_metric(arena->runtime,
                                        KEK_TRACE_METRIC_STATE_ARENA_RESET, 0);
        kek_trace_record_runtime_metric(arena->runtime,
                                        KEK_TRACE_METRIC_STATE_ARENA_HIGH_WATER,
                                        arena->high_water_mark);
    }
    arena->current = arena->first;
    arena->current_bytes = 0;
    for (KekStateArenaBlock* block = arena->first; block; block = block->next) {
        block->used = 0;
    }
}

static KekStateArenaBlock* state_arena_add_block(KekStateArena* arena,
                                                 size_t required) {
    if (!arena) {
        return NULL;
    }
    size_t capacity = required > KEK_STATE_ARENA_BLOCK_CAPACITY
                          ? required
                          : KEK_STATE_ARENA_BLOCK_CAPACITY;
    KekStateArenaBlock* block =
        (KekStateArenaBlock*)kek_trace_malloc(arena->runtime, sizeof(*block));
    if (!block) {
        return NULL;
    }
    memset(block, 0, sizeof(*block));
    block->data = (unsigned char*)kek_trace_malloc(arena->runtime, capacity);
    if (!block->data) {
        kek_trace_free(arena->runtime, block, sizeof(*block));
        return NULL;
    }
    block->capacity = capacity;
    if (!arena->first) {
        arena->first = block;
    }
    if (arena->current) {
        arena->current->next = block;
    }
    arena->current = block;
    return block;
}

void* kek_state_arena_alloc(KekStateArena* arena, size_t size, size_t alignment) {
    if (!arena || size == 0) {
        return NULL;
    }
    if (!arena->current) {
        if (!state_arena_add_block(arena, state_store_align_up(size, alignment))) {
            return NULL;
        }
    }
    size_t aligned = state_store_align_up(arena->current->used, alignment);
    if (aligned + size > arena->current->capacity) {
        if (!state_arena_add_block(arena, state_store_align_up(size, alignment))) {
            return NULL;
        }
        aligned = state_store_align_up(arena->current->used, alignment);
    }
    void* ptr = arena->current->data + aligned;
    arena->current->used = aligned + size;
    arena->current_bytes += size;
    if (arena->current_bytes > arena->high_water_mark) {
        arena->high_water_mark = arena->current_bytes;
    }
    return ptr;
}

size_t kek_state_arena_high_water_mark(const KekStateArena* arena) {
    return arena ? arena->high_water_mark : 0;
}

void kek_state_store_init(KekStateStore* store, KekRuntime* runtime) {
    kek_state_store_init_with_allocator(store, runtime, NULL);
}

void kek_state_store_init_with_allocator(KekStateStore* store, KekRuntime* runtime,
                                         KekStateArena* allocator) {
    if (!store) {
        return;
    }
    memset(store, 0, sizeof(*store));
    store->runtime = runtime;
    if (allocator) {
        store->allocator = allocator;
        store->owns_allocator = 0;
    } else {
        kek_state_arena_init(&store->arena, runtime);
        store->allocator = &store->arena;
        store->owns_allocator = 1;
    }
    store->owns_type_pool_records = 1;
    store->active_hook.trigger_state_slot = KEK_STATE_INVALID_ID;
}

static void state_slot_clear(KekStateStore* store, KekStateSlot* slot) {
    if (!slot) {
        return;
    }
    uint32_t generation = slot->generation;
    size_t pool_index = KEK_STATE_INVALID_ID;
    size_t size = slot->descriptor ? slot->descriptor->size : 0;
    if (slot->buffer_owned[0]) {
        kek_trace_free(store ? store->runtime : NULL, slot->buffers[0], size);
    }
    if (slot->buffer_owned[1]) {
        kek_trace_free(store ? store->runtime : NULL, slot->buffers[1], size);
    }
    memset(slot, 0, sizeof(*slot));
    slot->generation = generation;
    slot->pool_index = pool_index;
}

static unsigned char* state_store_alloc_buffer(KekStateStore* store, size_t size,
                                               size_t alignment,
                                               unsigned char* owned) {
    if (owned) {
        *owned = 0;
    }
    if (!store || size == 0) {
        return NULL;
    }
    void* ptr = kek_state_arena_alloc(store->allocator, size, alignment);
    if (ptr) {
        if (!store->owns_allocator && kek_trace_enabled(store->allocator->runtime)) {
            kek_trace_record_runtime_metric(
                store->allocator->runtime,
                KEK_TRACE_METRIC_HOOK_OVERLAY_DRAFT_BYTES, size);
            kek_trace_count_runtime_metric(store->allocator->runtime,
                                           KEK_TRACE_METRIC_HOOK_OVERLAY_ENTRY);
        }
        return (unsigned char*)ptr;
    }
    ptr = kek_trace_malloc(store->runtime, size);
    if (owned && ptr) {
        *owned = 1;
    }
    return (unsigned char*)ptr;
}

static int state_slot_ensure_buffer(KekStateStore* store, KekStateSlot* slot,
                                    size_t index) {
    if (!store || !slot || !slot->descriptor || index >= 2) {
        return 0;
    }
    if (slot->buffers[index]) {
        return 1;
    }
    slot->buffers[index] =
        state_store_alloc_buffer(store, slot->descriptor->size,
                                 slot->descriptor->alignment,
                                 &slot->buffer_owned[index]);
    return slot->buffers[index] != NULL;
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
    int should_snapshot =
        event_type == KEK_EVENT_STATE_DELETED ||
        (store->runtime && store->runtime->state_snapshots_enabled);
    if (should_snapshot &&
        slot->descriptor->size <= KEK_EVENT_STATE_SNAPSHOT_CAPACITY) {
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
    int should_snapshot =
        event_type == KEK_EVENT_STATE_DELETED ||
        (store->runtime && store->runtime->state_snapshots_enabled);
    if (should_snapshot && descriptor->size <= KEK_EVENT_STATE_SNAPSHOT_CAPACITY) {
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
    entry->buffer_owned[0] = slot->buffer_owned[0];
    entry->buffer_owned[1] = slot->buffer_owned[1];
    entry->active_index = slot->active_index;
    entry->pool_index = slot->pool_index;
    entry->version = slot->version;
    entry->pending_version = slot->version;
    entry->generation = slot->generation;
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
    if (store->owns_type_pool_records) {
        for (size_t i = 0; i < KEK_STATE_STORE_MAX_SLOTS; i++) {
            state_store_type_pool_destroy(store, &store->type_pools[i]);
        }
    }
    if (store->owns_allocator) {
        kek_state_arena_destroy(&store->arena);
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
            transaction_entry->buffer_owned[0] = store->slots[slot_id].buffer_owned[0];
            transaction_entry->buffer_owned[1] = store->slots[slot_id].buffer_owned[1];
            transaction_entry->owns_buffers = 1;
            store->slots[slot_id].buffers[0] = NULL;
            store->slots[slot_id].buffers[1] = NULL;
            store->slots[slot_id].buffer_owned[0] = 0;
            store->slots[slot_id].buffer_owned[1] = 0;
        }
    }

    KekStateSlot* slot = &store->slots[slot_id];
    uint32_t previous_generation = slot->generation;
    memset(slot, 0, sizeof(*slot));
    slot->pool_index = KEK_STATE_INVALID_ID;
    slot->descriptor = descriptor;
    slot->generation = previous_generation + 1;
    if (slot->generation == 0) {
        slot->generation = 1;
    }
    size_t pool_index = KEK_STATE_INVALID_ID;
    KekStateHandle handle =
        state_store_type_pool_add(store, descriptor, slot_id,
                                  slot->generation, &pool_index);
    if (handle == KEK_STATE_INVALID_ID) {
        state_slot_clear(store, slot);
        return KEK_STATE_INVALID_ID;
    }
    KekStateTypePool* pool = state_store_type_pool(store, descriptor->type_id, 0);
    slot->pool_index = pool_index;
    slot->buffers[0] = state_store_type_pool_record(pool, pool_index, 0);
    slot->buffers[1] = state_store_type_pool_record(pool, pool_index, 1);
    if (!slot->buffers[0] || !slot->buffers[1]) {
        state_store_type_pool_remove(store, descriptor->type_id, handle);
        state_slot_clear(store, slot);
        return KEK_STATE_INVALID_ID;
    }

    if (initial_state) {
        state_store_trace_copy(store, KEK_TRACE_METRIC_STATE_STORE_INIT_COPY,
                               slot->buffers[0], initial_state,
                               descriptor->size);
    } else if (descriptor->set_default) {
        descriptor->set_default(slot->buffers[0]);
    } else {
        memset(slot->buffers[0], 0, descriptor->size);
    }

    state_store_trace_copy(store, KEK_TRACE_METRIC_STATE_STORE_INIT_COPY,
                           slot->buffers[1], slot->buffers[0],
                           descriptor->size);
    if (!state_store_trace_check(store, descriptor->check, slot->buffers[0])) {
        state_store_type_pool_remove(store, descriptor->type_id, handle);
        state_slot_clear(store, slot);
        return KEK_STATE_INVALID_ID;
    }

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
    if (!state_store_publish_slot_event(store, slot, handle,
                                        KEK_EVENT_STATE_CREATED,
                                        KEK_EVENT_CHANGED_FIELDS_NONE)) {
        state_store_type_pool_remove(store, descriptor->type_id, handle);
        state_slot_clear(store, slot);
        while (store->slot_count > 0 && !store->slots[store->slot_count - 1].in_use) {
            store->slot_count--;
        }
        return KEK_STATE_INVALID_ID;
    }
    return handle;
}

size_t kek_state_store_add_default(KekStateStore* store,
                                   const KekStateDescriptor* descriptor) {
    return kek_state_store_add(store, descriptor, NULL);
}

int kek_state_store_remove(KekStateStore* store, KekStateHandle slot_id) {
    size_t slot_index = state_store_handle_index_checked(store, slot_id);
    KekStateSlot* slot = state_store_slot(store, slot_index);
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
        state_store_find_transaction_slot(transaction, slot_index);
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
        if (!state_store_transaction_record_slot(transaction, slot_index)) {
            return 0;
        }
        entry = state_store_transaction_slot(transaction, slot_index);
        if (entry->created && !entry->in_use) {
            state_store_type_pool_remove(store, slot->descriptor->type_id, slot_id);
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
    state_store_type_pool_remove(store, slot->descriptor->type_id, slot_id);
    state_slot_clear(store, slot);
    while (store->slot_count > 0 && !store->slots[store->slot_count - 1].in_use) {
        store->slot_count--;
    }
    return 1;
}

static KekStateSlot* state_store_slot(KekStateStore* store, size_t slot_id) {
    if (!store || slot_id == KEK_STATE_INVALID_ID ||
        slot_id >= store->slot_count || !store->slots[slot_id].in_use) {
        return NULL;
    }
    return &store->slots[slot_id];
}

static const KekStateSlot* state_store_slot_const(const KekStateStore* store,
                                                  size_t slot_id) {
    if (!store || slot_id == KEK_STATE_INVALID_ID ||
        slot_id >= store->slot_count || !store->slots[slot_id].in_use) {
        return NULL;
    }
    return &store->slots[slot_id];
}

void* kek_state_store_current(KekStateStore* store, KekStateHandle slot_id) {
    KekStateSlot* slot =
        state_store_slot(store, state_store_handle_index_checked(store, slot_id));
    return slot ? slot->buffers[slot->active_index] : NULL;
}

const void* kek_state_store_current_const(const KekStateStore* store,
                                          KekStateHandle slot_id) {
    const KekStateSlot* slot = state_store_slot_const(
        store, state_store_handle_index_checked(store, slot_id));
    return slot ? slot->buffers[slot->active_index] : NULL;
}

const KekStateDescriptor* kek_state_store_descriptor(const KekStateStore* store,
                                                     KekStateHandle slot_id) {
    const KekStateSlot* slot = state_store_slot_const(
        store, state_store_handle_index_checked(store, slot_id));
    return slot ? slot->descriptor : NULL;
}

uint64_t kek_state_store_version(const KekStateStore* store,
                                 KekStateHandle slot_id) {
    const KekStateSlot* slot = state_store_slot_const(
        store, state_store_handle_index_checked(store, slot_id));
    return slot ? slot->version : 0;
}

KekStateHandle kek_state_store_find_first(const KekStateStore* store,
                                          size_t state_type_id) {
    return kek_state_store_find_next(store, state_type_id, KEK_STATE_INVALID_ID);
}

KekStateHandle kek_state_store_find_next(const KekStateStore* store,
                                         size_t state_type_id,
                                         KekStateHandle after_slot_id) {
    const KekStateTypePool* pool =
        state_store_type_pool_const(store, state_type_id);
    if (!pool) {
        return KEK_STATE_INVALID_ID;
    }
    size_t start = 0;
    if (after_slot_id != KEK_STATE_INVALID_ID) {
        for (size_t i = 0; i < pool->count; i++) {
            if (pool->handles[i] == after_slot_id) {
                start = i + 1;
                break;
            }
        }
    }
    for (size_t i = start; i < pool->count; i++) {
        if (state_store_handle_index_checked(store, pool->handles[i]) !=
            KEK_STATE_INVALID_ID) {
            return pool->handles[i];
        }
    }
    return KEK_STATE_INVALID_ID;
}

int kek_state_store_update(KekStateStore* store, KekStateHandle slot_id,
                           KekStateUpdateFn update, void* context) {
    return kek_state_store_update_fields(store, slot_id, update, context,
                                         KEK_EVENT_CHANGED_FIELDS_UNKNOWN);
}

int kek_state_store_update_fields(KekStateStore* store, KekStateHandle slot_id,
                                  KekStateUpdateFn update, void* context,
                                  uint64_t changed_fields) {
    size_t slot_index = state_store_handle_index_checked(store, slot_id);
    KekStateSlot* slot = state_store_slot(store, slot_index);
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
        if (!state_store_transaction_record_slot(transaction, slot_index)) {
            return 0;
        }
        KekStateStoreTransactionSlot* entry =
            state_store_transaction_slot(transaction, slot_index);
        KekStateStoreTransactionSlot* visible_entry =
            state_store_find_transaction_slot(transaction->parent, slot_index);
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
                state_store_trace_copy(store,
                                       KEK_TRACE_METRIC_STATE_STORE_TRANSACTION_COPY,
                                       rollback_copy, draft,
                                       slot->descriptor->size);
                entry->buffers[0] = rollback_copy;
                entry->owns_buffers = 1;
                entry->pending_version = visible_entry->pending_version;
            } else {
                draft_index = slot->active_index == 0 ? 1u : 0u;
                if (!state_slot_ensure_buffer(store, slot, draft_index)) {
                    return 0;
                }
                draft = slot->buffers[draft_index];
                rollback_source = slot->buffers[slot->active_index];
                state_store_trace_copy(store,
                                       KEK_TRACE_METRIC_STATE_STORE_DRAFT_COPY,
                                       draft, rollback_source,
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
            state_store_trace_copy(store,
                                   KEK_TRACE_METRIC_STATE_STORE_TRANSACTION_COPY,
                                   rollback_copy, draft,
                                   slot->descriptor->size);
        }

        state_store_trace_update(store, update, draft, context);
        if (!state_store_trace_check(store, slot->descriptor->check, draft)) {
            if (rollback_copy) {
                state_store_trace_copy(store,
                                       KEK_TRACE_METRIC_STATE_STORE_ROLLBACK_COPY,
                                       draft, rollback_copy,
                                       slot->descriptor->size);
                kek_trace_free(store->runtime, rollback_copy,
                               slot->descriptor->size);
            } else if (rollback_source) {
                state_store_trace_copy(store,
                                       KEK_TRACE_METRIC_STATE_STORE_ROLLBACK_COPY,
                                       draft, rollback_source,
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
            state_store_trace_copy(store,
                                   KEK_TRACE_METRIC_STATE_STORE_ROLLBACK_COPY,
                                   draft, rollback_copy,
                                   slot->descriptor->size);
            if (entry->owns_buffers && entry->buffers[0] == rollback_copy) {
                entry->buffers[0] = NULL;
                entry->owns_buffers = 0;
            }
            kek_trace_free(store->runtime, rollback_copy, slot->descriptor->size);
        } else if (rollback_source) {
            state_store_trace_copy(store,
                                   KEK_TRACE_METRIC_STATE_STORE_ROLLBACK_COPY,
                                   draft, rollback_source,
                                   slot->descriptor->size);
            entry->dirty = 0;
            entry->draft_index = KEK_STATE_INVALID_ID;
        }
        return 0;
    }

    size_t inactive_index = slot->active_index == 0 ? 1u : 0u;
    if (!state_slot_ensure_buffer(store, slot, inactive_index)) {
        return 0;
    }
    void* current = slot->buffers[slot->active_index];
    void* draft = slot->buffers[inactive_index];
    state_store_trace_copy(store, KEK_TRACE_METRIC_STATE_STORE_DRAFT_COPY, draft,
                           current, slot->descriptor->size);

    state_store_trace_update(store, update, draft, context);
    if (!state_store_trace_check(store, slot->descriptor->check, draft)) {
        state_store_trace_copy(store, KEK_TRACE_METRIC_STATE_STORE_ROLLBACK_COPY,
                               draft, current, slot->descriptor->size);
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
    state_store_trace_copy(store, KEK_TRACE_METRIC_STATE_STORE_ROLLBACK_COPY,
                           draft, current, slot->descriptor->size);
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
            if (!kek_state_store_update_fields(store, updates[i].handle,
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
            if (updates[i].handle == updates[j].handle) {
                return 0;
            }
        }
        size_t slot_index = state_store_handle_index_checked(store,
                                                             updates[i].handle);
        KekStateSlot* slot = state_store_slot(store, slot_index);
        if (!slot || !slot->descriptor || !slot->descriptor->check) {
            return 0;
        }
        if (!state_store_write_allowed(store, slot)) {
            return 0;
        }
        slots[i] = slot;
        active_indices[i] = slot->active_index;
        inactive_indices[i] = slot->active_index == 0 ? 1u : 0u;
        if (!state_slot_ensure_buffer(store, slot, inactive_indices[i])) {
            return 0;
        }
        state_store_trace_copy(store, KEK_TRACE_METRIC_STATE_STORE_DRAFT_COPY,
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
                    store, KEK_TRACE_METRIC_STATE_STORE_ROLLBACK_COPY,
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
        if (!state_store_publish_slot_event(store, slots[i], updates[i].handle,
                                            KEK_EVENT_STATE_CHANGED,
                                            updates[i].changed_fields)) {
            for (size_t j = 0; j < update_count; j++) {
                slots[j]->active_index = active_indices[j];
                slots[j]->version--;
                state_store_trace_copy(store,
                                       KEK_TRACE_METRIC_STATE_STORE_ROLLBACK_COPY,
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
        state_store_trace_copy(store, KEK_TRACE_METRIC_STATE_STORE_ROLLBACK_COPY,
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
            if (entry->buffer_owned[0]) {
                kek_trace_free(transaction->store ? transaction->store->runtime : NULL,
                               entry->buffers[0], size);
            }
            if (entry->buffer_owned[1]) {
                kek_trace_free(transaction->store ? transaction->store->runtime : NULL,
                               entry->buffers[1], size);
            }
        }
        entry->buffers[0] = NULL;
        entry->buffers[1] = NULL;
        entry->buffer_owned[0] = 0;
        entry->buffer_owned[1] = 0;
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
                parent_entry->pool_index = entry->pool_index;
                parent_entry->version = entry->version;
                parent_entry->pending_version = entry->version;
                parent_entry->in_use = entry->in_use;
                parent_entry->recorded = 1;
                parent_entry->draft_index = KEK_STATE_INVALID_ID;
                if (entry->owns_buffers && entry->in_use &&
                    entry->buffers[0] && entry->buffers[1]) {
                    parent_entry->buffers[0] = entry->buffers[0];
                    parent_entry->buffers[1] = entry->buffers[1];
                    parent_entry->buffer_owned[0] = entry->buffer_owned[0];
                    parent_entry->buffer_owned[1] = entry->buffer_owned[1];
                    parent_entry->owns_buffers = 1;
                    entry->buffers[0] = NULL;
                    entry->buffers[1] = NULL;
                    entry->buffer_owned[0] = 0;
                    entry->buffer_owned[1] = 0;
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
            KekStateHandle handle =
                entry->descriptor
                    ? kek_state_handle_make(entry->descriptor->type_id,
                                            entry->pool_index,
                                            entry->generation)
                    : KEK_STATE_INVALID_ID;
            if (handle != KEK_STATE_INVALID_ID) {
                state_store_type_pool_remove(store, entry->descriptor->type_id,
                                             handle);
            }
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
        KekStateHandle handle = state_store_slot_handle(store, i);
        if (handle != KEK_STATE_INVALID_ID && store->slots[i].descriptor) {
            state_store_type_pool_remove(store, store->slots[i].descriptor->type_id,
                                         handle);
        }
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
            KekStateHandle current_handle = state_store_slot_handle(store, i);
            if (current_handle != KEK_STATE_INVALID_ID && slot->descriptor) {
                state_store_type_pool_remove(store, slot->descriptor->type_id,
                                             current_handle);
            }
            state_slot_clear(store, slot);
            memset(slot, 0, sizeof(*slot));
            slot->descriptor = snapshot->descriptor;
            slot->buffers[0] = snapshot->buffers[0];
            slot->buffers[1] = snapshot->buffers[1];
            slot->buffer_owned[0] = snapshot->buffer_owned[0];
            slot->buffer_owned[1] = snapshot->buffer_owned[1];
            slot->active_index = snapshot->active_index;
            slot->pool_index = snapshot->pool_index;
            slot->version = snapshot->version;
            slot->generation = snapshot->generation;
            slot->in_use = snapshot->in_use;
            snapshot->buffers[0] = NULL;
            snapshot->buffers[1] = NULL;
            snapshot->buffer_owned[0] = 0;
            snapshot->buffer_owned[1] = 0;
            snapshot->owns_buffers = 0;
        } else if (snapshot->owns_buffers && snapshot->dirty &&
                   snapshot->buffers[0]) {
            state_store_trace_copy(store, KEK_TRACE_METRIC_STATE_STORE_ROLLBACK_COPY,
                                   slot->buffers[snapshot->draft_index],
                                   snapshot->buffers[0],
                                   snapshot->descriptor->size);
        } else {
            if (snapshot->created && !snapshot->in_use) {
                KekStateHandle handle = state_store_slot_handle(store, i);
                if (handle != KEK_STATE_INVALID_ID && slot->descriptor) {
                    state_store_type_pool_remove(store, slot->descriptor->type_id,
                                                 handle);
                }
                state_slot_clear(store, slot);
                memset(slot, 0, sizeof(*slot));
                continue;
            }
            slot->descriptor = snapshot->descriptor;
            slot->active_index = snapshot->active_index;
            slot->pool_index = snapshot->pool_index;
            slot->version = snapshot->version;
            slot->generation = snapshot->generation;
            slot->in_use = snapshot->in_use;
        }
    }
    state_store_transaction_clear(transaction);
}
