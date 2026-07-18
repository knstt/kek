#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "runtime/hook.h"
#include "runtime/runtime.h"
#include "runtime/standard_io.h"
#include "runtime/state_storage.h"
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
    size_t counter_slot;
    size_t input_slot;
    size_t output_slot;
    size_t timer_slot;
    size_t stream_data_events;
    size_t hook_runs;
    size_t self_hook_runs;
    size_t text_hook_runs;
    size_t failing_hook_runs;
    int write_stream_id;
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

static const KekStateDescriptor counter_descriptor = {
    SMOKE_STATE_COUNTER, "SmokeCounter", sizeof(SmokeCounterState),
    default_counter, check_counter, NULL};
static const KekStateDescriptor text_descriptor = {
    SMOKE_STATE_TEXT, "SmokeText", sizeof(SmokeTextState),
    default_text, check_text, NULL};
static const KekStateDescriptor timer_descriptor = {
    SMOKE_STATE_TIMER, "SmokeTimer", sizeof(SmokeTimerState),
    default_timer, check_timer, NULL};

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

static int set_text_state(KekStateStore* store, size_t slot_id,
                          const char* data, size_t len) {
    TextUpdate update = {data ? data : "", data ? len : 0};
    return kek_state_store_update_fields(store, slot_id, update_text, &update,
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
    KekHookDescriptor descriptors[] = {
        {"self counter", KEK_EVENT_STATE_CHANGED, SMOKE_STATE_COUNTER,
         app.counter_slot, 1ull << 0, NULL, 0, counter_writes, 1,
         self_counter_hook},
        {"text field", KEK_EVENT_STATE_CHANGED, SMOKE_STATE_TEXT,
         app.input_slot, 1ull << 1, NULL, 0, NULL, 0, text_field_hook},
        {"failing counter", KEK_EVENT_STATE_CHANGED, SMOKE_STATE_COUNTER,
         app.counter_slot, 1ull << 0, NULL, 0, NULL, 0, failing_counter_hook},
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
        timer_writes, 1, timer_hook};
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

int main(void) {
    int result = run_smoke();
    if (result == 0) {
        result = run_behavior_checks();
    }
    if (result == 0) {
        puts("runtime smoke passed");
    } else {
        puts("runtime smoke failed");
    }
    return result;
}
