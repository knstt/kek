#include "runtime_stress_support.h"

int OnPacketSeq(KekHookContext* context) {
    return runtime_stress_note_telemetry(context, 0, 0, 0, 0.5);
}
