#include "runtime_stress_support.h"

#include <stdlib.h>
#include <string.h>

typedef struct TelemetryDelta {
    uint64_t total_events;
    uint64_t hook_hits;
    uint64_t stream_bytes;
    uint64_t timer_ticks;
    int active_agent_delta;
    double load_delta;
} TelemetryDelta;

typedef struct AuditDelta {
    uint64_t last_event;
    uint64_t deleted_count;
    uint64_t batch_count;
    uint64_t eof_count;
    uint64_t error_count;
} AuditDelta;

static void update_telemetry(void* draft, void* context) {
    Telemetry* telemetry = (Telemetry*)draft;
    const TelemetryDelta* delta = (const TelemetryDelta*)context;
    if (!telemetry || !delta) {
        return;
    }

    telemetry->total_events += delta->total_events;
    telemetry->hook_hits += delta->hook_hits;
    telemetry->stream_bytes += delta->stream_bytes;
    telemetry->timer_ticks += delta->timer_ticks;
    telemetry->load += delta->load_delta;
    telemetry->active_agents += delta->active_agent_delta;
    if (telemetry->active_agents < 0) {
        telemetry->active_agents = 0;
    }
    if (telemetry->active_agents > 128) {
        telemetry->active_agents = 128;
    }
}

static void update_audit(void* draft, void* context) {
    AuditLog* audit = (AuditLog*)draft;
    const AuditDelta* delta = (const AuditDelta*)context;
    if (!audit || !delta) {
        return;
    }

    audit->last_event += delta->last_event;
    audit->deleted_count += delta->deleted_count;
    audit->batch_count += delta->batch_count;
    audit->eof_count += delta->eof_count;
    audit->error_count += delta->error_count;
}

static RuntimeStressApp* stress_app(KekHookContext* context) {
    return context ? (RuntimeStressApp*)context->app_context : NULL;
}

static void stress_note_readonly_active(RuntimeStressApp* app, uint64_t active) {
    uint64_t previous = atomic_load(&app->readonly_max_active);
    while (active > previous &&
           !atomic_compare_exchange_weak(&app->readonly_max_active, &previous,
                                         active)) {
    }
}

static void stress_note_write_benchmark_active(RuntimeStressApp* app,
                                               uint64_t active) {
    uint64_t previous = atomic_load(&app->write_benchmark_max_active);
    while (active > previous &&
           !atomic_compare_exchange_weak(&app->write_benchmark_max_active,
                                         &previous, active)) {
    }
}

static uint64_t stress_readonly_work_iterations(void) {
    const char* value = getenv("KEK_STRESS_READONLY_WORK");
    if (!value || value[0] == '\0') {
        return 40000;
    }

    char* end = NULL;
    unsigned long long parsed = strtoull(value, &end, 10);
    if (end == value || (end && *end != '\0')) {
        return 40000;
    }
    return (uint64_t)parsed;
}

static uint64_t stress_write_work_iterations(void) {
    const char* value = getenv("KEK_STRESS_WRITE_WORK");
    if (!value || value[0] == '\0') {
        return 40000;
    }

    char* end = NULL;
    unsigned long long parsed = strtoull(value, &end, 10);
    if (end == value || (end && *end != '\0')) {
        return 40000;
    }
    return (uint64_t)parsed;
}

int runtime_stress_note_telemetry(KekHookContext* context, uint64_t stream_bytes,
                                  uint64_t timer_ticks, int active_agent_delta,
                                  double load_delta) {
    if (!context || !context->state_store) {
        return 0;
    }

    RuntimeStressApp* app = stress_app(context);
    if (app) {
        app->hook_calls++;
    }

    size_t slot_id = runtime_stress_state_telemetry_first(context->state_store);
    if (slot_id == KEK_STATE_INVALID_ID) {
        return 0;
    }

    TelemetryDelta delta = {1, 1, stream_bytes, timer_ticks, active_agent_delta,
                            load_delta};
    uint64_t fields = KEK_STATE_TYPE_TELEMETRY_FIELD_TOTAL_EVENTS |
                      KEK_STATE_TYPE_TELEMETRY_FIELD_HOOK_HITS;
    if (stream_bytes > 0) {
        fields |= KEK_STATE_TYPE_TELEMETRY_FIELD_STREAM_BYTES;
    }
    if (timer_ticks > 0) {
        fields |= KEK_STATE_TYPE_TELEMETRY_FIELD_TIMER_TICKS;
    }
    if (active_agent_delta != 0) {
        fields |= KEK_STATE_TYPE_TELEMETRY_FIELD_ACTIVE_AGENTS;
    }
    if (load_delta != 0.0) {
        fields |= KEK_STATE_TYPE_TELEMETRY_FIELD_LOAD;
    }
    return kek_state_store_update_fields(context->state_store, slot_id,
                                         update_telemetry, &delta, fields);
}

int runtime_stress_note_audit(KekHookContext* context, uint64_t deleted_delta,
                              uint64_t batch_delta, uint64_t eof_delta,
                              uint64_t error_delta) {
    if (!context || !context->state_store) {
        return 0;
    }

    RuntimeStressApp* app = stress_app(context);
    if (app) {
        app->hook_calls++;
    }

    size_t slot_id = runtime_stress_state_audit_log_first(context->state_store);
    if (slot_id == KEK_STATE_INVALID_ID) {
        return 0;
    }

    AuditDelta delta = {1, deleted_delta, batch_delta, eof_delta, error_delta};
    uint64_t fields = KEK_STATE_TYPE_AUDIT_LOG_FIELD_LAST_EVENT;
    if (deleted_delta > 0) {
        fields |= KEK_STATE_TYPE_AUDIT_LOG_FIELD_DELETED_COUNT;
    }
    if (batch_delta > 0) {
        fields |= KEK_STATE_TYPE_AUDIT_LOG_FIELD_BATCH_COUNT;
    }
    if (eof_delta > 0) {
        fields |= KEK_STATE_TYPE_AUDIT_LOG_FIELD_EOF_COUNT;
    }
    if (error_delta > 0) {
        fields |= KEK_STATE_TYPE_AUDIT_LOG_FIELD_ERROR_COUNT;
    }
    return kek_state_store_update_fields(context->state_store, slot_id,
                                         update_audit, &delta, fields);
}

int runtime_stress_fail_clock_phase(KekHookContext* context) {
    RuntimeStressApp* app = stress_app(context);
    if (!app || !app->force_clock_phase_failure) {
        return runtime_stress_note_audit(context, 0, 0, 0, 0);
    }

    app->hook_failures++;
    if (!runtime_stress_note_audit(context, 0, 0, 0, 0)) {
        return 0;
    }
    return 0;
}

int runtime_stress_check_forbidden_audit_write(KekHookContext* context) {
    RuntimeStressApp* app = stress_app(context);
    if (!app || !app->check_forbidden_write) {
        return 1;
    }

    size_t slot_id = runtime_stress_state_audit_log_first(context->state_store);
    if (slot_id == KEK_STATE_INVALID_ID) {
        return 0;
    }

    AuditDelta delta = {1, 0, 0, 0, 0};
    int allowed = kek_state_store_update_fields(
        context->state_store, slot_id, update_audit, &delta,
        KEK_STATE_TYPE_AUDIT_LOG_FIELD_LAST_EVENT);
    if (allowed) {
        return 0;
    }
    app->forbidden_write_checks++;
    return 1;
}

int runtime_stress_readonly_probe(KekHookContext* context, uint64_t salt) {
    RuntimeStressApp* app = stress_app(context);
    if (!context || !context->state_store || !app) {
        return 0;
    }

    size_t size = 0;
    const Agent* agent = (const Agent*)kek_hook_event_state(context, &size);
    if (!agent || size != sizeof(*agent)) {
        return 1;
    }

    size_t clock_slot =
        runtime_stress_state_simulation_clock_first(context->state_store);
    const SimulationClock* clock =
        (const SimulationClock*)kek_state_store_current_const(context->state_store,
                                                             clock_slot);
    uint64_t seed = salt ^ agent->id ^ agent->flags;
    if (clock) {
        seed ^= clock->tick + ((uint64_t)(uint32_t)clock->phase << 32);
    }

    uint64_t active = atomic_fetch_add(&app->readonly_active, 1) + 1;
    stress_note_readonly_active(app, active);

    uint64_t digest = seed ? seed : salt;
    uint64_t iterations = stress_readonly_work_iterations();
    for (uint64_t i = 0; i < iterations; i++) {
        digest ^= digest << 13;
        digest ^= digest >> 7;
        digest ^= digest << 17;
        digest += (uint64_t)(agent->energy * 1000.0) + i + salt;
    }

    atomic_fetch_add(&app->readonly_digest, digest);
    atomic_fetch_add(&app->readonly_hook_calls, 1);
    atomic_fetch_sub(&app->readonly_active, 1);
    return 1;
}

void runtime_stress_note_write_benchmark(KekHookContext* context, int merge,
                                         uint64_t salt) {
    RuntimeStressApp* app = stress_app(context);
    if (!app) {
        return;
    }

    uint64_t active = atomic_fetch_add(&app->write_benchmark_active, 1) + 1;
    stress_note_write_benchmark_active(app, active);

    uint64_t digest = salt ? salt : 0x9E3779B97F4A7C15ull;
    uint64_t iterations = stress_write_work_iterations();
    for (uint64_t i = 0; i < iterations; i++) {
        digest ^= digest << 13;
        digest ^= digest >> 7;
        digest ^= digest << 17;
        digest += i + (merge ? 0xD1B54A32D192ED03ull : 0x94D049BB133111EBull);
    }

    atomic_fetch_add(&app->write_benchmark_digest, digest);
    if (merge) {
        atomic_fetch_add(&app->write_benchmark_merge_hooks, 1);
    } else {
        atomic_fetch_add(&app->write_benchmark_exact_hooks, 1);
    }
    atomic_fetch_sub(&app->write_benchmark_active, 1);
}
