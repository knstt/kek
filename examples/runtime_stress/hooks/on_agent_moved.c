#include "runtime_stress_support.h"

int OnAgentMoved(KekHookContext* context) {
    size_t size = 0;
    const Agent* agent = (const Agent*)kek_hook_event_state(context, &size);
    double load = size == sizeof(*agent) ? agent->energy * 0.001 : 0.1;
    return runtime_stress_note_telemetry(context, 0, 0, 0, load);
}
