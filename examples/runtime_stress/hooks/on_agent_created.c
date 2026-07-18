#include "runtime_stress_support.h"

int OnAgentCreated(KekHookContext* context) {
    return runtime_stress_note_telemetry(context, 0, 0, 1, 0.0);
}
