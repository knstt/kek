#include "runtime_stress_support.h"

int OnAgentActive(KekHookContext* context) {
    size_t size = 0;
    const Agent* agent = (const Agent*)kek_hook_event_state(context, &size);
    int delta = size == sizeof(*agent) && agent->active ? 1 : -1;
    return runtime_stress_note_telemetry(context, 0, 0, delta, 0.0);
}
