#include "runtime_stress_support.h"

int OnPacketCreated(KekHookContext* context) {
    return runtime_stress_note_telemetry(context, 0, 0, 0, 0.1);
}
