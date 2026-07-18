#include "runtime_stress_support.h"

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
