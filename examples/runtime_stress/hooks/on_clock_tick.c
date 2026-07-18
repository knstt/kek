#include "runtime_stress_support.h"

int OnClockTick(KekHookContext* context) {
    size_t size = 0;
    const SimulationClock* clock =
        (const SimulationClock*)kek_hook_event_state(context, &size);
    double load = size == sizeof(*clock) ? (double)(clock->tick % 17u) : 1.0;
    return runtime_stress_note_telemetry(context, 0, 0, 0, load);
}
