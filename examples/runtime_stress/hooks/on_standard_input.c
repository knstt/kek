#include "runtime_stress_support.h"

int OnStandardInput(KekHookContext* context) {
    size_t size = 0;
    const StandardInput* input =
        (const StandardInput*)kek_hook_event_state(context, &size);
    uint64_t bytes = size == sizeof(*input) ? input->input.len : 0;
    return runtime_stress_note_telemetry(context, bytes, 0, 0, 0.0);
}
