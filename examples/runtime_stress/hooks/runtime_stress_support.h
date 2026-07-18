#ifndef RUNTIME_STRESS_SUPPORT_H
#define RUNTIME_STRESS_SUPPORT_H

#include <stddef.h>
#include <stdint.h>

#include "runtime/standard_io.h"
#include "runtime_stress_state_hooks.h"

typedef struct RuntimeStressApp {
    uint64_t subscriber_events[KEK_EVENT_TYPE_COUNT];
    uint64_t hook_calls;
    uint64_t hook_failures;
    uint64_t forbidden_write_checks;
    Runtime_stress_stateRuntime* generated;
    KekStandardTextBridge* input_bridge;
    int force_clock_phase_failure;
    int check_forbidden_write;
} RuntimeStressApp;

int runtime_stress_note_telemetry(KekHookContext* context, uint64_t stream_bytes,
                                  uint64_t timer_ticks, int active_agent_delta,
                                  double load_delta);
int runtime_stress_note_audit(KekHookContext* context, uint64_t deleted_delta,
                              uint64_t batch_delta, uint64_t eof_delta,
                              uint64_t error_delta);
int runtime_stress_fail_clock_phase(KekHookContext* context);
int runtime_stress_check_forbidden_audit_write(KekHookContext* context);

#endif
