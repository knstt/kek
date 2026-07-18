#include "runtime_stress_support.h"

int OnFastTimer(KekHookContext* context) {
    size_t size = 0;
    const Timer* timer = (const Timer*)kek_hook_event_state(context, &size);
    if (size == sizeof(*timer) && timer->tick >= 3) {
        kek_runtime_request_quit(context->runtime);
    }
    return runtime_stress_note_telemetry(context, 0, 1, 0, 0.0);
}
