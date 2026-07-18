#include "runtime_stress_support.h"

int OnControlLabel(KekHookContext* context) {
    return runtime_stress_check_forbidden_audit_write(context) &&
           runtime_stress_note_telemetry(context, 0, 0, 0, 0.75);
}
