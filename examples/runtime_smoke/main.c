#define _POSIX_C_SOURCE 200809L

#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "runtime/hook.h"
#include "runtime/runtime.h"
#include "runtime/standard_io.h"
#include "runtime/state_store.h"
#include "runtime/stream.h"
#include "runtime/timer.h"

enum {
    SMOKE_STATE_COUNTER = 1,
    SMOKE_STATE_TEXT = 2,
    SMOKE_STATE_TIMER = 3
};

typedef struct SmokeCounterState {
    int value;
} SmokeCounterState;

typedef struct SmokeTextState {
    char text[128];
    size_t len;
} SmokeTextState;

typedef struct SmokeTimerState {
    uint64_t tick;
    uint32_t interval_ms;
    bool enabled;
} SmokeTimerState;

typedef struct SmokeApp {
    KekRuntime* runtime;
    KekStateStore* store;
    KekStateHandle counter_slot;
    KekStateHandle second_counter_slot;
    KekStateHandle input_slot;
    KekStateHandle output_slot;
    KekStateHandle timer_slot;
    size_t stream_data_events;
    size_t hook_runs;
    size_t self_hook_runs;
    size_t text_hook_runs;
    size_t failing_hook_runs;
    size_t double_text_hook_runs;
    size_t text_snapshot_first_events;
    size_t text_snapshot_second_events;
    size_t nested_child_hook_runs;
    int saw_committed_text_during_hook;
    int write_stream_id;
    atomic_int parallel_active;
    atomic_int parallel_max_active;
    atomic_int parallel_runs;
} SmokeApp;

typedef struct TextUpdate {
    const char* data;
    size_t len;
} TextUpdate;

static int check_counter(const void* state) {
    const SmokeCounterState* counter = (const SmokeCounterState*)state;
    return counter && counter->value >= 0 && counter->value < 100;
}

static int check_text(const void* state) {
    const SmokeTextState* text = (const SmokeTextState*)state;
    return text && text->len < sizeof(text->text) && text->text[text->len] == '\0';
}

static int check_timer(const void* state) {
    const SmokeTimerState* timer = (const SmokeTimerState*)state;
    return timer && timer->interval_ms <= 1000;
}

static void default_counter(void* state) {
    ((SmokeCounterState*)state)->value = 0;
}

static void default_text(void* state) {
    SmokeTextState* text = (SmokeTextState*)state;
    text->text[0] = '\0';
    text->len = 0;
}

static void default_timer(void* state) {
    SmokeTimerState* timer = (SmokeTimerState*)state;
    timer->tick = 0;
    timer->interval_ms = 0;
    timer->enabled = false;
}

static int merge_text_fields(void* target, const void* source, uint64_t fields) {
    SmokeTextState* target_text = (SmokeTextState*)target;
    const SmokeTextState* source_text = (const SmokeTextState*)source;
    if (!target_text || !source_text) {
        return 0;
    }
    if ((fields & (1ull << 0)) != 0) {
        memcpy(target_text->text, source_text->text, sizeof(target_text->text));
    }
    if ((fields & (1ull << 1)) != 0) {
        target_text->len = source_text->len;
    }
    return 1;
}

static const KekStateDescriptor counter_descriptor = {
    .type_id = SMOKE_STATE_COUNTER,
    .name = "SmokeCounter",
    .size = sizeof(SmokeCounterState),
    .alignment = _Alignof(SmokeCounterState),
    .set_default = default_counter,
    .check = check_counter,
};
static const KekStateDescriptor text_descriptor = {
    .type_id = SMOKE_STATE_TEXT,
    .name = "SmokeText",
    .size = sizeof(SmokeTextState),
    .alignment = _Alignof(SmokeTextState),
    .set_default = default_text,
    .check = check_text,
    .merge_fields = merge_text_fields,
};
static const KekStateDescriptor timer_descriptor = {
    .type_id = SMOKE_STATE_TIMER,
    .name = "SmokeTimer",
    .size = sizeof(SmokeTimerState),
    .alignment = _Alignof(SmokeTimerState),
    .set_default = default_timer,
    .check = check_timer,
};

static void update_text(void* draft, void* context) {
    SmokeTextState* text = (SmokeTextState*)draft;
    const TextUpdate* update = (const TextUpdate*)context;
    size_t to_copy = update->len < sizeof(text->text) - 1
                         ? update->len
                         : sizeof(text->text) - 1;
    memcpy(text->text, update->data, to_copy);
    text->text[to_copy] = '\0';
    text->len = to_copy;
}

static int set_text_state(KekStateStore* store, KekStateHandle handle,
                          const char* data, size_t len) {
    TextUpdate update = {data ? data : "", data ? len : 0};
    return kek_state_store_update_fields(store, handle, update_text, &update,
                                         1ull << 0 | 1ull << 1);
}

static void add_to_counter(void* draft, void* context) {
    SmokeCounterState* counter = (SmokeCounterState*)draft;
    int* delta = (int*)context;
    counter->value += *delta;
}

static void set_counter_value(void* draft, void* context) {
    SmokeCounterState* counter = (SmokeCounterState*)draft;
    int* value = (int*)context;
    counter->value = *value;
}

static void set_text_bytes_only(void* draft, void* context) {
    SmokeTextState* text = (SmokeTextState*)draft;
    const char* value = (const char*)context;
    memset(text->text, 0, sizeof(text->text));
    memcpy(text->text, value, strlen(value));
}

static void set_text_len_only(void* draft, void* context) {
    SmokeTextState* text = (SmokeTextState*)draft;
    size_t* value = (size_t*)context;
    text->len = *value;
}

static int stream_event_handler(const KekEvent* event, void* context) {
    SmokeApp* app = (SmokeApp*)context;
    if (event->type != KEK_EVENT_STREAM_DATA) {
        return 1;
    }

    app->stream_data_events++;
    return set_text_state(app->store, app->input_slot, event->data, event->data_len);
}

static int timer_hook(KekHookContext* context) {
    SmokeApp* app = (SmokeApp*)context->app_context;
    size_t snapshot_size = 0;
    const SmokeTimerState* snapshot =
        (const SmokeTimerState*)kek_hook_event_state(context, &snapshot_size);
    if (!snapshot || snapshot_size != sizeof(*snapshot) || snapshot->tick == 0) {
        return 1;
    }

    app->hook_runs++;
    int delta = 1;
    if (!kek_state_store_update_fields(context->state_store, app->counter_slot,
                                       add_to_counter, &delta,
                                       1ull << 0)) {
        return 0;
    }
    kek_runtime_request_quit(context->runtime);
    return 1;
}

static int generic_event_counter(const KekEvent* event, void* context) {
    size_t* count = (size_t*)context;
    if (event->type == KEK_EVENT_STATE_CHANGED) {
        (*count)++;
    }
    return 1;
}

static int self_counter_hook(KekHookContext* context) {
    SmokeApp* app = (SmokeApp*)context->app_context;
    const SmokeCounterState* counter =
        (const SmokeCounterState*)kek_hook_event_state(context, NULL);
    if (!counter || counter->value != 1) {
        return 1;
    }
    app->self_hook_runs++;
    int value = 2;
    return kek_state_store_update_fields(context->state_store, app->counter_slot,
                                         set_counter_value, &value, 1ull << 0);
}

static int text_field_hook(KekHookContext* context) {
    SmokeApp* app = (SmokeApp*)context->app_context;
    app->text_hook_runs++;
    return 1;
}

static int text_snapshot_counter(const KekEvent* event, void* context) {
    SmokeApp* app = (SmokeApp*)context;
    if (event->type != KEK_EVENT_STATE_CHANGED ||
        event->state_type_id != SMOKE_STATE_TEXT || !event->has_state_snapshot ||
        event->state_snapshot_size != sizeof(SmokeTextState)) {
        return 1;
    }
    const SmokeTextState* text =
        (const SmokeTextState*)event->state_snapshot.data;
    if (strcmp(text->text, "first") == 0) {
        app->text_snapshot_first_events++;
    }
    if (strcmp(text->text, "second") == 0) {
        app->text_snapshot_second_events++;
    }
    return 1;
}

static int double_text_hook(KekHookContext* context) {
    SmokeApp* app = (SmokeApp*)context->app_context;
    app->double_text_hook_runs++;
    if (!set_text_state(context->state_store, app->input_slot, "first", 5)) {
        return 0;
    }
    const SmokeTextState* current =
        (const SmokeTextState*)kek_state_store_current_const(context->state_store,
                                                            app->input_slot);
    if (current && strcmp(current->text, "seed") == 0) {
        app->saw_committed_text_during_hook = 1;
    }
    return set_text_state(context->state_store, app->input_slot, "second", 6);
}

static int failing_counter_hook(KekHookContext* context) {
    SmokeApp* app = (SmokeApp*)context->app_context;
    app->failing_hook_runs++;
    int value = 4;
    if (!kek_state_store_update_fields(context->state_store, app->counter_slot,
                                       set_counter_value, &value, 1ull << 0)) {
        return 0;
    }
    return 0;
}

static int failing_double_text_hook(KekHookContext* context) {
    SmokeApp* app = (SmokeApp*)context->app_context;
    app->failing_hook_runs++;
    if (!set_text_state(context->state_store, app->input_slot, "first", 5) ||
        !set_text_state(context->state_store, app->input_slot, "second", 6)) {
        return 0;
    }
    return 0;
}

static int failing_create_text_hook(KekHookContext* context) {
    SmokeApp* app = (SmokeApp*)context->app_context;
    app->failing_hook_runs++;
    return kek_state_store_add_default(context->state_store, &text_descriptor) !=
           KEK_STATE_INVALID_ID
               ? 0
               : 1;
}

static int failing_delete_text_hook(KekHookContext* context) {
    SmokeApp* app = (SmokeApp*)context->app_context;
    app->failing_hook_runs++;
    if (!kek_state_store_remove(context->state_store, app->input_slot)) {
        return 0;
    }
    return 0;
}

static int failing_delete_reuse_text_hook(KekHookContext* context) {
    SmokeApp* app = (SmokeApp*)context->app_context;
    app->failing_hook_runs++;
    if (!kek_state_store_remove(context->state_store, app->input_slot)) {
        return 0;
    }
    KekStateHandle reused_slot =
        kek_state_store_add_default(context->state_store, &text_descriptor);
    if (reused_slot != app->input_slot) {
        return 0;
    }
    return set_text_state(context->state_store, reused_slot, "replacement", 11) ? 0
                                                                               : 1;
}

static int nested_parent_text_hook(KekHookContext* context) {
    SmokeApp* app = (SmokeApp*)context->app_context;
    if (!set_text_state(context->state_store, app->input_slot, "first", 5)) {
        return 0;
    }
    if (kek_event_dispatch_pending(kek_runtime_events(context->runtime))) {
        return 0;
    }
    return set_text_state(context->state_store, app->input_slot, "second", 6);
}

static int nested_failing_text_hook(KekHookContext* context) {
    SmokeApp* app = (SmokeApp*)context->app_context;
    const SmokeTextState* snapshot =
        (const SmokeTextState*)kek_hook_event_state(context, NULL);
    if (!snapshot || strcmp(snapshot->text, "first") != 0) {
        return 1;
    }
    app->nested_child_hook_runs++;
    if (!set_text_state(context->state_store, app->input_slot, "child", 5)) {
        return 0;
    }
    return 0;
}

static void note_parallel_active(SmokeApp* app, int active) {
    int previous = atomic_load(&app->parallel_max_active);
    while (active > previous &&
           !atomic_compare_exchange_weak(&app->parallel_max_active, &previous,
                                         active)) {
    }
}

static int readonly_parallel_hook(KekHookContext* context) {
    SmokeApp* app = (SmokeApp*)context->app_context;
    const SmokeCounterState* counter =
        (const SmokeCounterState*)kek_hook_event_state(context, NULL);
    if (!counter || counter->value <= 0) {
        return 1;
    }

    int active = atomic_fetch_add(&app->parallel_active, 1) + 1;
    note_parallel_active(app, active);
    usleep(120000);
    atomic_fetch_add(&app->parallel_runs, 1);
    atomic_fetch_sub(&app->parallel_active, 1);
    return 1;
}

static int parallel_write_counter_a_hook(KekHookContext* context) {
    SmokeApp* app = (SmokeApp*)context->app_context;
    int active = atomic_fetch_add(&app->parallel_active, 1) + 1;
    note_parallel_active(app, active);
    usleep(120000);
    int value = 11;
    int ok = kek_state_store_update_fields(context->state_store, app->counter_slot,
                                           set_counter_value, &value, 1ull << 0);
    atomic_fetch_add(&app->parallel_runs, 1);
    atomic_fetch_sub(&app->parallel_active, 1);
    return ok;
}

static int parallel_write_counter_b_hook(KekHookContext* context) {
    SmokeApp* app = (SmokeApp*)context->app_context;
    int active = atomic_fetch_add(&app->parallel_active, 1) + 1;
    note_parallel_active(app, active);
    usleep(120000);
    int value = 22;
    int ok = kek_state_store_update_fields(context->state_store,
                                           app->second_counter_slot,
                                           set_counter_value, &value, 1ull << 0);
    atomic_fetch_add(&app->parallel_runs, 1);
    atomic_fetch_sub(&app->parallel_active, 1);
    return ok;
}

static int parallel_merge_text_hook(KekHookContext* context) {
    SmokeApp* app = (SmokeApp*)context->app_context;
    int active = atomic_fetch_add(&app->parallel_active, 1) + 1;
    note_parallel_active(app, active);
    usleep(120000);
    int ok = kek_state_store_update_fields(context->state_store, app->input_slot,
                                           set_text_bytes_only, "merge", 1ull << 0);
    atomic_fetch_add(&app->parallel_runs, 1);
    atomic_fetch_sub(&app->parallel_active, 1);
    return ok;
}

static int parallel_merge_len_hook(KekHookContext* context) {
    SmokeApp* app = (SmokeApp*)context->app_context;
    int active = atomic_fetch_add(&app->parallel_active, 1) + 1;
    note_parallel_active(app, active);
    usleep(120000);
    size_t len = 5;
    int ok = kek_state_store_update_fields(context->state_store, app->input_slot,
                                           set_text_len_only, &len, 1ull << 1);
    atomic_fetch_add(&app->parallel_runs, 1);
    atomic_fetch_sub(&app->parallel_active, 1);
    return ok;
}

static size_t count_state_type(KekStateStore* store, size_t state_type_id) {
    size_t count = 0;
    KekStateHandle handle = KEK_STATE_INVALID_ID;
    while ((handle = kek_state_store_find_next(store, state_type_id, handle)) !=
           KEK_STATE_INVALID_ID) {
        count++;
    }
    return count;
}

static int run_handle_reuse_check(void) {
    KekRuntime runtime;
    KekStateStore store;
    kek_runtime_init(&runtime);
    kek_state_store_init(&store, &runtime);

    KekStateHandle first =
        kek_state_store_add_default(&store, &counter_descriptor);
    if (first == KEK_STATE_INVALID_ID ||
        !kek_event_dispatch_pending(kek_runtime_events(&runtime))) {
        return 1;
    }
    if (!kek_state_store_remove(&store, first) ||
        !kek_event_dispatch_pending(kek_runtime_events(&runtime))) {
        return 1;
    }
    KekStateHandle reused =
        kek_state_store_add_default(&store, &counter_descriptor);
    if (reused == KEK_STATE_INVALID_ID ||
        !kek_event_dispatch_pending(kek_runtime_events(&runtime))) {
        return 1;
    }
    KekStateHandle text =
        kek_state_store_add_default(&store, &text_descriptor);
    if (text == KEK_STATE_INVALID_ID ||
        !kek_event_dispatch_pending(kek_runtime_events(&runtime))) {
        return 1;
    }
    KekStateHandle second_text =
        kek_state_store_add_default(&store, &text_descriptor);
    if (second_text == KEK_STATE_INVALID_ID ||
        !kek_event_dispatch_pending(kek_runtime_events(&runtime))) {
        return 1;
    }
    const unsigned char* text_record =
        (const unsigned char*)kek_state_store_current_const(&store, text);
    const unsigned char* second_text_record =
        (const unsigned char*)kek_state_store_current_const(&store, second_text);

    int value = 7;
    int ok = first != reused &&
             kek_state_handle_index(first) == kek_state_handle_index(reused) &&
             kek_state_handle_index(text) == kek_state_handle_index(reused) &&
             kek_state_handle_index(second_text) ==
                 kek_state_handle_index(text) + 1 &&
             kek_state_handle_type_id(text) != kek_state_handle_type_id(reused) &&
             kek_state_handle_generation(first) !=
                 kek_state_handle_generation(reused) &&
             text_record && second_text_record &&
             second_text_record == text_record + sizeof(SmokeTextState) &&
             kek_state_store_current_const(&store, first) == NULL &&
             !kek_state_store_update_fields(&store, first, set_counter_value,
                                            &value, 1ull << 0) &&
             kek_state_store_update_fields(&store, reused, set_counter_value,
                                           &value, 1ull << 0) &&
             count_state_type(&store, SMOKE_STATE_COUNTER) == 1 &&
             count_state_type(&store, SMOKE_STATE_TEXT) == 2 &&
             kek_state_store_find_first(&store, SMOKE_STATE_COUNTER) == reused;

    kek_state_store_destroy(&store);
    kek_runtime_destroy(&runtime);
    return ok ? 0 : 1;
}

static int run_threading_mode_check(const char* thread_setting,
                                    int expect_parallel) {
    if (setenv("KEK_RUNTIME_THREADS", thread_setting, 1) != 0) {
        return 1;
    }

    KekRuntime runtime;
    KekStateStore store;
    KekHookRegistry hooks;
    SmokeApp app;

    kek_runtime_init(&runtime);
    kek_state_store_init(&store, &runtime);
    memset(&app, 0, sizeof(app));
    atomic_init(&app.parallel_active, 0);
    atomic_init(&app.parallel_max_active, 0);
    atomic_init(&app.parallel_runs, 0);
    app.runtime = &runtime;
    app.store = &store;
    app.counter_slot = kek_state_store_add_default(&store, &counter_descriptor);
    if (app.counter_slot == KEK_STATE_INVALID_ID ||
        !kek_event_dispatch_pending(kek_runtime_events(&runtime))) {
        return 1;
    }

    size_t counter_reads[] = {SMOKE_STATE_COUNTER};
    KekHookDescriptor descriptors[] = {
        {"parallel readonly a", KEK_EVENT_STATE_CHANGED, SMOKE_STATE_COUNTER,
         app.counter_slot, 1ull << 0, counter_reads, 1, NULL, 0,
         readonly_parallel_hook, NULL, 0,
         KEK_HOOK_SCHEDULING_NEEDS_EVENT_STATE},
        {"parallel readonly b", KEK_EVENT_STATE_CHANGED, SMOKE_STATE_COUNTER,
         app.counter_slot, 1ull << 0, counter_reads, 1, NULL, 0,
         readonly_parallel_hook, NULL, 0,
         KEK_HOOK_SCHEDULING_NEEDS_EVENT_STATE},
    };

    kek_hook_registry_init(&hooks, &runtime, &store, &app);
    if (!kek_hook_registry_add_many(&hooks, descriptors, 2)) {
        return 1;
    }
    kek_hook_registry_attach(&hooks);

    int value = 1;
    if (!kek_state_store_update_fields(&store, app.counter_slot,
                                       set_counter_value, &value, 1ull << 0) ||
        !kek_event_dispatch_pending(kek_runtime_events(&runtime))) {
        return 1;
    }

    int runs = atomic_load(&app.parallel_runs);
    int max_active = atomic_load(&app.parallel_max_active);
    int thread_count = (int)kek_runtime_thread_count(&runtime);
    int ok = runs == 2 && thread_count == (expect_parallel ? 3 : 1) &&
             max_active == (expect_parallel ? 2 : 1);

    kek_hook_registry_detach(&hooks);
    kek_state_store_destroy(&store);
    kek_runtime_destroy(&runtime);
    unsetenv("KEK_RUNTIME_THREADS");
    return ok ? 0 : 1;
}

static int run_parallel_write_check(void) {
    if (setenv("KEK_RUNTIME_THREADS", "3", 1) != 0) {
        return 1;
    }

    KekRuntime runtime;
    KekStateStore store;
    KekHookRegistry hooks;
    SmokeApp app;

    kek_runtime_init(&runtime);
    kek_state_store_init(&store, &runtime);
    memset(&app, 0, sizeof(app));
    atomic_init(&app.parallel_active, 0);
    atomic_init(&app.parallel_max_active, 0);
    atomic_init(&app.parallel_runs, 0);
    app.runtime = &runtime;
    app.store = &store;
    app.counter_slot = kek_state_store_add_default(&store, &counter_descriptor);
    app.second_counter_slot =
        kek_state_store_add_default(&store, &counter_descriptor);
    app.input_slot = kek_state_store_add_default(&store, &text_descriptor);
    if (app.counter_slot == KEK_STATE_INVALID_ID ||
        app.second_counter_slot == KEK_STATE_INVALID_ID ||
        app.input_slot == KEK_STATE_INVALID_ID ||
        !kek_event_dispatch_pending(kek_runtime_events(&runtime))) {
        return 1;
    }

    KekHookAccess write_a_accesses[] = {
        {KEK_HOOK_ACCESS_READ, SMOKE_STATE_TEXT, app.input_slot,
         KEK_HOOK_ACCESS_SCOPE_EXACT_SLOT, 1ull << 0},
        {KEK_HOOK_ACCESS_WRITE, SMOKE_STATE_COUNTER, app.counter_slot,
         KEK_HOOK_ACCESS_SCOPE_EXACT_SLOT, 1ull << 0},
    };
    KekHookAccess write_b_accesses[] = {
        {KEK_HOOK_ACCESS_READ, SMOKE_STATE_TEXT, app.input_slot,
         KEK_HOOK_ACCESS_SCOPE_EXACT_SLOT, 1ull << 0},
        {KEK_HOOK_ACCESS_WRITE, SMOKE_STATE_COUNTER, app.second_counter_slot,
         KEK_HOOK_ACCESS_SCOPE_EXACT_SLOT, 1ull << 0},
    };
    KekHookDescriptor descriptors[] = {
        {"parallel write a", KEK_EVENT_STATE_CHANGED, SMOKE_STATE_TEXT,
         app.input_slot, 1ull << 0, NULL, 0, NULL, 0,
         parallel_write_counter_a_hook, write_a_accesses, 2,
         KEK_HOOK_SCHEDULING_ALLOW_PARALLEL_WRITES},
        {"parallel write b", KEK_EVENT_STATE_CHANGED, SMOKE_STATE_TEXT,
         app.input_slot, 1ull << 0, NULL, 0, NULL, 0,
         parallel_write_counter_b_hook, write_b_accesses, 2,
         KEK_HOOK_SCHEDULING_ALLOW_PARALLEL_WRITES},
    };

    kek_hook_registry_init(&hooks, &runtime, &store, &app);
    if (!kek_hook_registry_add_many(&hooks, descriptors, 2)) {
        return 1;
    }
    kek_hook_registry_attach(&hooks);

    if (!set_text_state(&store, app.input_slot, "parallel-write", 14) ||
        !kek_event_dispatch_pending(kek_runtime_events(&runtime))) {
        return 1;
    }

    const SmokeCounterState* first =
        (const SmokeCounterState*)kek_state_store_current_const(&store,
                                                                app.counter_slot);
    const SmokeCounterState* second =
        (const SmokeCounterState*)kek_state_store_current_const(
            &store, app.second_counter_slot);
    int ok = first && second && first->value == 11 && second->value == 22 &&
             atomic_load(&app.parallel_runs) == 2 &&
             atomic_load(&app.parallel_max_active) == 2;

    kek_hook_registry_detach(&hooks);
    kek_state_store_destroy(&store);
    kek_runtime_destroy(&runtime);
    unsetenv("KEK_RUNTIME_THREADS");
    return ok ? 0 : 1;
}

static int run_parallel_field_merge_check(void) {
    if (setenv("KEK_RUNTIME_THREADS", "3", 1) != 0) {
        return 1;
    }

    KekRuntime runtime;
    KekStateStore store;
    KekHookRegistry hooks;
    SmokeApp app;

    kek_runtime_init(&runtime);
    kek_state_store_init(&store, &runtime);
    memset(&app, 0, sizeof(app));
    atomic_init(&app.parallel_active, 0);
    atomic_init(&app.parallel_max_active, 0);
    atomic_init(&app.parallel_runs, 0);
    app.runtime = &runtime;
    app.store = &store;
    app.counter_slot = kek_state_store_add_default(&store, &counter_descriptor);
    app.input_slot = kek_state_store_add_default(&store, &text_descriptor);
    if (app.counter_slot == KEK_STATE_INVALID_ID ||
        app.input_slot == KEK_STATE_INVALID_ID ||
        !kek_event_dispatch_pending(kek_runtime_events(&runtime))) {
        return 1;
    }
    if (!set_text_state(&store, app.input_slot, "aaaaa", 5) ||
        !kek_event_dispatch_pending(kek_runtime_events(&runtime))) {
        return 1;
    }

    KekHookAccess text_accesses[] = {
        {KEK_HOOK_ACCESS_READ, SMOKE_STATE_COUNTER, app.counter_slot,
         KEK_HOOK_ACCESS_SCOPE_EXACT_SLOT, 1ull << 0},
        {KEK_HOOK_ACCESS_WRITE, SMOKE_STATE_TEXT, app.input_slot,
         KEK_HOOK_ACCESS_SCOPE_EXACT_SLOT, 1ull << 0},
    };
    KekHookAccess len_accesses[] = {
        {KEK_HOOK_ACCESS_READ, SMOKE_STATE_COUNTER, app.counter_slot,
         KEK_HOOK_ACCESS_SCOPE_EXACT_SLOT, 1ull << 0},
        {KEK_HOOK_ACCESS_WRITE, SMOKE_STATE_TEXT, app.input_slot,
         KEK_HOOK_ACCESS_SCOPE_EXACT_SLOT, 1ull << 1},
    };
    uint32_t flags = KEK_HOOK_SCHEDULING_ALLOW_PARALLEL_WRITES |
                     KEK_HOOK_SCHEDULING_FIELD_MERGE_SAFE;
    KekHookDescriptor descriptors[] = {
        {"parallel merge text", KEK_EVENT_STATE_CHANGED, SMOKE_STATE_COUNTER,
         app.counter_slot, 1ull << 0, NULL, 0, NULL, 0,
         parallel_merge_text_hook, text_accesses, 2, flags},
        {"parallel merge len", KEK_EVENT_STATE_CHANGED, SMOKE_STATE_COUNTER,
         app.counter_slot, 1ull << 0, NULL, 0, NULL, 0,
         parallel_merge_len_hook, len_accesses, 2, flags},
    };

    kek_hook_registry_init(&hooks, &runtime, &store, &app);
    if (!kek_hook_registry_add_many(&hooks, descriptors, 2)) {
        return 1;
    }
    kek_hook_registry_attach(&hooks);

    int value = 1;
    if (!kek_state_store_update_fields(&store, app.counter_slot,
                                       set_counter_value, &value, 1ull << 0) ||
        !kek_event_dispatch_pending(kek_runtime_events(&runtime))) {
        return 1;
    }

    const SmokeTextState* text =
        (const SmokeTextState*)kek_state_store_current_const(&store,
                                                            app.input_slot);
    int ok = text && strcmp(text->text, "merge") == 0 && text->len == 5 &&
             atomic_load(&app.parallel_runs) == 2 &&
             atomic_load(&app.parallel_max_active) == 2;

    kek_hook_registry_detach(&hooks);
    kek_state_store_destroy(&store);
    kek_runtime_destroy(&runtime);
    unsetenv("KEK_RUNTIME_THREADS");
    return ok ? 0 : 1;
}

static int run_threading_checks(void) {
    if (run_threading_mode_check("1", 0) != 0) {
        return 1;
    }
    if (run_threading_mode_check("3", 1) != 0) {
        return 1;
    }
    if (run_parallel_write_check() != 0) {
        return 1;
    }
    return run_parallel_field_merge_check();
}

static int run_behavior_checks(void) {
    KekRuntime runtime;
    KekStateStore store;
    KekHookRegistry hooks;
    SmokeApp app;

    kek_runtime_init(&runtime);
    kek_state_store_init(&store, &runtime);
    memset(&app, 0, sizeof(app));
    app.runtime = &runtime;
    app.store = &store;

    app.counter_slot = kek_state_store_add_default(&store, &counter_descriptor);
    app.input_slot = kek_state_store_add_default(&store, &text_descriptor);
    if (app.counter_slot == KEK_STATE_INVALID_ID ||
        app.input_slot == KEK_STATE_INVALID_ID) {
        return 1;
    }
    if (!kek_event_dispatch_pending(kek_runtime_events(&runtime))) {
        return 1;
    }

    size_t duplicate_subscription_events = 0;
    if (!kek_event_subscribe(kek_runtime_events(&runtime), KEK_EVENT_STATE_CHANGED,
                             generic_event_counter,
                             &duplicate_subscription_events) ||
        !kek_event_subscribe(kek_runtime_events(&runtime), KEK_EVENT_STATE_CHANGED,
                             generic_event_counter,
                             &duplicate_subscription_events)) {
        return 1;
    }
    KekEvent duplicate_event;
    memset(&duplicate_event, 0, sizeof(duplicate_event));
    duplicate_event.type = KEK_EVENT_STATE_CHANGED;
    if (!kek_event_publish(kek_runtime_events(&runtime), &duplicate_event) ||
        !kek_event_dispatch_pending(kek_runtime_events(&runtime)) ||
        duplicate_subscription_events != 1) {
        return 1;
    }
    if (!kek_event_unsubscribe(kek_runtime_events(&runtime), KEK_EVENT_STATE_CHANGED,
                               generic_event_counter,
                               &duplicate_subscription_events)) {
        return 1;
    }
    if (!kek_event_publish(kek_runtime_events(&runtime), &duplicate_event) ||
        !kek_event_dispatch_pending(kek_runtime_events(&runtime)) ||
        duplicate_subscription_events != 1) {
        return 1;
    }

    size_t counter_writes[] = {SMOKE_STATE_COUNTER};
    size_t text_writes[] = {SMOKE_STATE_TEXT};
    KekHookDescriptor descriptors[] = {
        {"self counter", KEK_EVENT_STATE_CHANGED, SMOKE_STATE_COUNTER,
         app.counter_slot, 1ull << 0, NULL, 0, counter_writes, 1,
         self_counter_hook, NULL, 0, KEK_HOOK_SCHEDULING_NEEDS_EVENT_STATE},
        {"text field", KEK_EVENT_STATE_CHANGED, SMOKE_STATE_TEXT,
         app.input_slot, 1ull << 1, NULL, 0, NULL, 0, text_field_hook, NULL, 0, 0},
        {"failing counter", KEK_EVENT_STATE_CHANGED, SMOKE_STATE_COUNTER,
         app.counter_slot, 1ull << 0, NULL, 0, NULL, 0, failing_counter_hook, NULL, 0, 0},
        {"failing create text", KEK_EVENT_STATE_CHANGED, SMOKE_STATE_COUNTER,
         app.counter_slot, 1ull << 0, NULL, 0, text_writes, 1,
         failing_create_text_hook, NULL, 0, 0},
        {"double text", KEK_EVENT_STATE_CHANGED, SMOKE_STATE_COUNTER,
         app.counter_slot, 1ull << 0, NULL, 0, text_writes, 1,
         double_text_hook, NULL, 0, 0},
        {"failing double text", KEK_EVENT_STATE_CHANGED, SMOKE_STATE_COUNTER,
         app.counter_slot, 1ull << 0, NULL, 0, text_writes, 1,
         failing_double_text_hook, NULL, 0, 0},
        {"failing delete text", KEK_EVENT_STATE_CHANGED, SMOKE_STATE_COUNTER,
         app.counter_slot, 1ull << 0, NULL, 0, text_writes, 1,
         failing_delete_text_hook, NULL, 0, 0},
        {"failing delete reuse text", KEK_EVENT_STATE_CHANGED, SMOKE_STATE_COUNTER,
         app.counter_slot, 1ull << 0, NULL, 0, text_writes, 1,
         failing_delete_reuse_text_hook, NULL, 0, 0},
        {"nested parent text", KEK_EVENT_STATE_CHANGED, SMOKE_STATE_COUNTER,
         app.counter_slot, 1ull << 0, NULL, 0, text_writes, 1,
         nested_parent_text_hook, NULL, 0, 0},
        {"nested failing text", KEK_EVENT_STATE_CHANGED, SMOKE_STATE_TEXT,
         app.input_slot, 1ull << 0 | 1ull << 1, NULL, 0, text_writes, 1,
         nested_failing_text_hook, NULL, 0,
         KEK_HOOK_SCHEDULING_NEEDS_EVENT_STATE},
    };
    kek_hook_registry_init(&hooks, &runtime, &store, &app);
    if (!kek_hook_registry_add(&hooks, &descriptors[0]) ||
        !kek_hook_registry_add(&hooks, &descriptors[1])) {
        return 1;
    }
    kek_hook_registry_attach(&hooks);

    int value = 1;
    if (!kek_state_store_update_fields(&store, app.counter_slot,
                                       set_counter_value, &value, 1ull << 0) ||
        !kek_event_dispatch_pending(kek_runtime_events(&runtime))) {
        return 1;
    }
    const SmokeCounterState* counter =
        (const SmokeCounterState*)kek_state_store_current_const(&store,
                                                                app.counter_slot);
    if (!counter || counter->value != 2 || app.self_hook_runs != 1) {
        return 1;
    }

    if (!set_text_state(&store, app.input_slot, "abc", 3) ||
        !kek_event_dispatch_pending(kek_runtime_events(&runtime)) ||
        app.text_hook_runs != 1) {
        return 1;
    }
    if (!kek_state_store_update_fields(&store, app.input_slot,
                                       update_text,
                                       &(TextUpdate){"def", 3}, 1ull << 0) ||
        !kek_event_dispatch_pending(kek_runtime_events(&runtime)) ||
        app.text_hook_runs != 1) {
        return 1;
    }

    value = 101;
    if (kek_state_store_update_fields(&store, app.counter_slot,
                                      set_counter_value, &value, 1ull << 0)) {
        return 1;
    }
    counter = (const SmokeCounterState*)kek_state_store_current_const(
        &store, app.counter_slot);
    if (!counter || counter->value != 2) {
        return 1;
    }

    kek_hook_registry_detach(&hooks);
    kek_hook_registry_init(&hooks, &runtime, &store, &app);
    if (!kek_hook_registry_add(&hooks, &descriptors[4]) ||
        !kek_event_subscribe(kek_runtime_events(&runtime), KEK_EVENT_STATE_CHANGED,
                             text_snapshot_counter, &app)) {
        return 1;
    }
    runtime.state_snapshots_enabled = 1;
    kek_hook_registry_attach(&hooks);
    if (!set_text_state(&store, app.input_slot, "seed", 4) ||
        !kek_event_dispatch_pending(kek_runtime_events(&runtime))) {
        return 1;
    }
    app.text_snapshot_first_events = 0;
    app.text_snapshot_second_events = 0;
    value = 5;
    if (!kek_state_store_update_fields(&store, app.counter_slot,
                                       set_counter_value, &value, 1ull << 0) ||
        !kek_event_dispatch_pending(kek_runtime_events(&runtime)) ||
        app.double_text_hook_runs != 1 ||
        !app.saw_committed_text_during_hook ||
        app.text_snapshot_first_events != 1 ||
        app.text_snapshot_second_events != 1) {
        return 1;
    }
    const SmokeTextState* text =
        (const SmokeTextState*)kek_state_store_current_const(&store, app.input_slot);
    if (!text || strcmp(text->text, "second") != 0) {
        return 1;
    }
    if (!kek_event_unsubscribe(kek_runtime_events(&runtime),
                               KEK_EVENT_STATE_CHANGED,
                               text_snapshot_counter, &app)) {
        return 1;
    }
    runtime.state_snapshots_enabled = 0;

    kek_hook_registry_detach(&hooks);
    kek_hook_registry_init(&hooks, &runtime, &store, &app);
    if (!kek_hook_registry_add(&hooks, &descriptors[2])) {
        return 1;
    }
    kek_hook_registry_attach(&hooks);
    value = 3;
    if (!kek_state_store_update_fields(&store, app.counter_slot,
                                       set_counter_value, &value, 1ull << 0) ||
        kek_event_dispatch_pending(kek_runtime_events(&runtime)) ||
        app.failing_hook_runs != 1) {
        return 1;
    }
    counter = (const SmokeCounterState*)kek_state_store_current_const(
        &store, app.counter_slot);
    if (!counter || counter->value != 3 ||
        kek_event_has_pending(kek_runtime_events(&runtime))) {
        return 1;
    }

    kek_hook_registry_detach(&hooks);
    kek_hook_registry_init(&hooks, &runtime, &store, &app);
    if (!kek_hook_registry_add(&hooks, &descriptors[5])) {
        return 1;
    }
    kek_hook_registry_attach(&hooks);
    if (!set_text_state(&store, app.input_slot, "stable", 6) ||
        !kek_event_dispatch_pending(kek_runtime_events(&runtime))) {
        return 1;
    }
    value = 6;
    if (!kek_state_store_update_fields(&store, app.counter_slot,
                                       set_counter_value, &value, 1ull << 0) ||
        kek_event_dispatch_pending(kek_runtime_events(&runtime)) ||
        kek_event_has_pending(kek_runtime_events(&runtime))) {
        return 1;
    }
    text = (const SmokeTextState*)kek_state_store_current_const(&store,
                                                               app.input_slot);
    if (!text || strcmp(text->text, "stable") != 0) {
        return 1;
    }

    kek_hook_registry_detach(&hooks);
    kek_hook_registry_init(&hooks, &runtime, &store, &app);
    if (!kek_hook_registry_add(&hooks, &descriptors[3])) {
        return 1;
    }
    kek_hook_registry_attach(&hooks);
    size_t text_count_before = count_state_type(&store, SMOKE_STATE_TEXT);
    value = 4;
    if (!kek_state_store_update_fields(&store, app.counter_slot,
                                       set_counter_value, &value, 1ull << 0) ||
        kek_event_dispatch_pending(kek_runtime_events(&runtime)) ||
        count_state_type(&store, SMOKE_STATE_TEXT) != text_count_before ||
        kek_event_has_pending(kek_runtime_events(&runtime))) {
        return 1;
    }

    kek_hook_registry_detach(&hooks);
    kek_hook_registry_init(&hooks, &runtime, &store, &app);
    if (!kek_hook_registry_add(&hooks, &descriptors[6])) {
        return 1;
    }
    kek_hook_registry_attach(&hooks);
    value = 7;
    if (!kek_state_store_update_fields(&store, app.counter_slot,
                                       set_counter_value, &value, 1ull << 0) ||
        kek_event_dispatch_pending(kek_runtime_events(&runtime)) ||
        kek_state_store_current_const(&store, app.input_slot) == NULL ||
        count_state_type(&store, SMOKE_STATE_TEXT) != text_count_before ||
        kek_event_has_pending(kek_runtime_events(&runtime))) {
        return 1;
    }

    kek_hook_registry_detach(&hooks);
    kek_hook_registry_init(&hooks, &runtime, &store, &app);
    if (!kek_hook_registry_add(&hooks, &descriptors[7])) {
        return 1;
    }
    kek_hook_registry_attach(&hooks);
    if (!set_text_state(&store, app.input_slot, "original", 8) ||
        !kek_event_dispatch_pending(kek_runtime_events(&runtime))) {
        return 1;
    }
    value = 8;
    if (!kek_state_store_update_fields(&store, app.counter_slot,
                                       set_counter_value, &value, 1ull << 0) ||
        kek_event_dispatch_pending(kek_runtime_events(&runtime)) ||
        count_state_type(&store, SMOKE_STATE_TEXT) != text_count_before ||
        kek_event_has_pending(kek_runtime_events(&runtime))) {
        return 1;
    }
    text = (const SmokeTextState*)kek_state_store_current_const(&store,
                                                               app.input_slot);
    if (!text || strcmp(text->text, "original") != 0) {
        return 1;
    }

    kek_hook_registry_detach(&hooks);
    kek_hook_registry_init(&hooks, &runtime, &store, &app);
    if (!kek_hook_registry_add(&hooks, &descriptors[8]) ||
        !kek_hook_registry_add(&hooks, &descriptors[9])) {
        return 1;
    }
    kek_hook_registry_attach(&hooks);
    app.nested_child_hook_runs = 0;
    value = 9;
    if (!kek_state_store_update_fields(&store, app.counter_slot,
                                       set_counter_value, &value, 1ull << 0) ||
        !kek_event_dispatch_pending(kek_runtime_events(&runtime)) ||
        app.nested_child_hook_runs != 1 ||
        kek_event_has_pending(kek_runtime_events(&runtime))) {
        return 1;
    }
    text = (const SmokeTextState*)kek_state_store_current_const(&store,
                                                               app.input_slot);
    if (!text || strcmp(text->text, "second") != 0) {
        return 1;
    }

    kek_hook_registry_detach(&hooks);
    kek_state_store_destroy(&store);
    kek_runtime_destroy(&runtime);
    return 0;
}

static int run_smoke(void) {
    KekRuntime runtime;
    KekStateStore store;
    KekHookRegistry hooks;
    SmokeApp app;
    size_t state_changed_events = 0;
    int input_pipe[2];
    int output_pipe[2];
    char standard_output_buffer[128];
    KekStandardTextBridge output_bridge;

    if (pipe(input_pipe) != 0 || pipe(output_pipe) != 0) {
        return 1;
    }

    kek_runtime_init(&runtime);
    kek_state_store_init(&store, &runtime);

    memset(&app, 0, sizeof(app));
    app.runtime = &runtime;
    app.store = &store;
    app.write_stream_id = -1;

    app.counter_slot = kek_state_store_add_default(&store, &counter_descriptor);
    app.input_slot = kek_state_store_add_default(&store, &text_descriptor);
    app.output_slot = kek_state_store_add_default(&store, &text_descriptor);
    app.timer_slot = kek_state_store_add_default(&store, &timer_descriptor);
    if (app.counter_slot == KEK_STATE_INVALID_ID ||
        app.input_slot == KEK_STATE_INVALID_ID ||
        app.output_slot == KEK_STATE_INVALID_ID ||
        app.timer_slot == KEK_STATE_INVALID_ID) {
        return 1;
    }

    int read_stream_id = kek_runtime_register_stream(&runtime, input_pipe[0],
                                                     KEK_STREAM_READ, 1);
    app.write_stream_id = kek_runtime_register_stream(&runtime, output_pipe[1],
                                                      KEK_STREAM_WRITE, 1);
    int timer_id = kek_runtime_register_timer(&runtime, &store, app.timer_slot, 1);
    if (read_stream_id < 0 || app.write_stream_id < 0 || timer_id < 0) {
        return 1;
    }

    KekStream* output_stream = kek_runtime_get_stream(&runtime,
                                                      (size_t)app.write_stream_id);
    kek_standard_text_bridge_init(&output_bridge, &store, app.output_slot,
                                  standard_output_buffer,
                                  sizeof(standard_output_buffer), set_text_state);
    if (kek_standard_output_write(output_stream, &output_bridge,
                                  "smoke-output", strlen("smoke-output")) == 0) {
        return 1;
    }

    if (write(input_pipe[1], "smoke-input", strlen("smoke-input")) < 0) {
        return 1;
    }
    close(input_pipe[1]);

    kek_event_subscribe(kek_runtime_events(&runtime), KEK_EVENT_STREAM_DATA,
                        stream_event_handler, &app);
    kek_event_subscribe(kek_runtime_events(&runtime), KEK_EVENT_STATE_CHANGED,
                        generic_event_counter, &state_changed_events);

    size_t timer_writes[] = {SMOKE_STATE_COUNTER};
    KekHookDescriptor timer_descriptor_hook = {
        "timer increments counter", KEK_EVENT_STATE_CHANGED, SMOKE_STATE_TIMER,
        app.timer_slot, KEK_EVENT_CHANGED_FIELDS_UNKNOWN, NULL, 0,
        timer_writes, 1, timer_hook, NULL, 0,
        KEK_HOOK_SCHEDULING_NEEDS_EVENT_STATE};
    kek_hook_registry_init(&hooks, &runtime, &store, &app);
    if (!kek_hook_registry_add(&hooks, &timer_descriptor_hook)) {
        return 1;
    }
    kek_hook_registry_attach(&hooks);

    if (kek_runtime_enable_raw_mode(&runtime, -1) != 0) {
        return 1;
    }
    if (kek_runtime_run(&runtime) != 0) {
        return 1;
    }

    const SmokeCounterState* counter =
        (const SmokeCounterState*)kek_state_store_current_const(&store,
                                                                app.counter_slot);
    const SmokeTextState* input =
        (const SmokeTextState*)kek_state_store_current_const(&store, app.input_slot);
    const SmokeTextState* output =
        (const SmokeTextState*)kek_state_store_current_const(&store, app.output_slot);

    int ok = counter && input && output && counter->value == 1 &&
             strcmp(input->text, "smoke-input") == 0 &&
             strcmp(output->text, "smoke-output") == 0 &&
             app.stream_data_events == 1 && app.hook_runs == 1 &&
             state_changed_events > 0;

    kek_hook_registry_detach(&hooks);
    kek_state_store_destroy(&store);
    kek_runtime_destroy(&runtime);
    close(output_pipe[0]);
    return ok ? 0 : 1;
}

static int trace_file_contains(const char* path, const char* text) {
    FILE* file = fopen(path, "r");
    if (!file) {
        return 0;
    }

    char buffer[4096];
    size_t count = fread(buffer, 1, sizeof(buffer) - 1, file);
    fclose(file);
    buffer[count] = '\0';
    return strstr(buffer, text) != NULL;
}

static int run_tracing_checks(void) {
    char runtime_csv[128];
    char hooks_csv[128];
    snprintf(runtime_csv, sizeof(runtime_csv), "/tmp/kek_trace_%ld_runtime.csv",
             (long)getpid());
    snprintf(hooks_csv, sizeof(hooks_csv), "/tmp/kek_trace_%ld_hooks.csv",
             (long)getpid());
    unlink(runtime_csv);
    unlink(hooks_csv);

    if (setenv("KEK_TRACE_RUNTIME_CSV", runtime_csv, 1) != 0 ||
        setenv("KEK_TRACE_HOOKS_CSV", hooks_csv, 1) != 0 ||
        setenv("KEK_RUNTIME_THREADS", "3", 1) != 0) {
        return 1;
    }

    KekRuntime runtime;
    KekStateStore store;
    KekHookRegistry hooks;
    SmokeApp app;

    kek_runtime_init(&runtime);
    kek_state_store_init(&store, &runtime);
    memset(&app, 0, sizeof(app));
    atomic_init(&app.parallel_active, 0);
    atomic_init(&app.parallel_max_active, 0);
    atomic_init(&app.parallel_runs, 0);
    app.runtime = &runtime;
    app.store = &store;

    app.counter_slot = kek_state_store_add_default(&store, &counter_descriptor);
    if (app.counter_slot == KEK_STATE_INVALID_ID) {
        return 1;
    }
    if (!kek_event_dispatch_pending(kek_runtime_events(&runtime))) {
        return 1;
    }

    size_t counter_reads[] = {SMOKE_STATE_COUNTER};
    KekHookDescriptor descriptors[] = {
        {"trace counter hook a", KEK_EVENT_STATE_CHANGED, SMOKE_STATE_COUNTER,
         app.counter_slot, 1ull << 0, counter_reads, 1, NULL, 0,
         readonly_parallel_hook, NULL, 0,
         KEK_HOOK_SCHEDULING_NEEDS_EVENT_STATE},
        {"trace counter hook b", KEK_EVENT_STATE_CHANGED, SMOKE_STATE_COUNTER,
         app.counter_slot, 1ull << 0, counter_reads, 1, NULL, 0,
         readonly_parallel_hook, NULL, 0,
         KEK_HOOK_SCHEDULING_NEEDS_EVENT_STATE},
    };
    kek_hook_registry_init(&hooks, &runtime, &store, &app);
    if (!kek_hook_registry_add_many(&hooks, descriptors, 2)) {
        return 1;
    }
    kek_hook_registry_attach(&hooks);

    int value = 1;
    if (!kek_state_store_update_fields(&store, app.counter_slot,
                                       set_counter_value, &value, 1ull << 0) ||
        !kek_event_dispatch_pending(kek_runtime_events(&runtime)) ||
        atomic_load(&app.parallel_runs) != 2) {
        return 1;
    }

    kek_hook_registry_detach(&hooks);
    kek_state_store_destroy(&store);
    kek_runtime_destroy(&runtime);
    unsetenv("KEK_TRACE_RUNTIME_CSV");
    unsetenv("KEK_TRACE_HOOKS_CSV");
    unsetenv("KEK_RUNTIME_THREADS");

    int ok = trace_file_contains(runtime_csv, "metric,count,total_ns") &&
             trace_file_contains(runtime_csv, "event_publish") &&
             trace_file_contains(runtime_csv, "runtime_memory") &&
             trace_file_contains(hooks_csv, "hook,event_type,state_type_id") &&
             trace_file_contains(hooks_csv, "trace counter hook a") &&
             trace_file_contains(hooks_csv, "trace counter hook b");

    unlink(runtime_csv);
    unlink(hooks_csv);
    return ok ? 0 : 1;
}

int main(void) {
    int result = run_smoke();
    if (result == 0) {
        result = run_handle_reuse_check();
        if (result != 0) {
            puts("runtime smoke failed: handle reuse");
        }
    }
    if (result == 0) {
        result = run_behavior_checks();
        if (result != 0) {
            puts("runtime smoke failed: behavior");
        }
    }
    if (result == 0) {
        result = run_threading_checks();
        if (result != 0) {
            puts("runtime smoke failed: threading");
        }
    }
    if (result == 0) {
        result = run_tracing_checks();
        if (result != 0) {
            puts("runtime smoke failed: tracing");
        }
    }
    if (result == 0) {
        puts("runtime smoke passed");
    } else {
        puts("runtime smoke failed");
    }
    return result;
}
