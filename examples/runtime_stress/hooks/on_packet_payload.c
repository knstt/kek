#include "runtime_stress_support.h"

int OnPacketPayload(KekHookContext* context) {
    size_t size = 0;
    const Packet* packet = (const Packet*)kek_hook_event_state(context, &size);
    uint64_t bytes = size == sizeof(*packet) ? packet->payload.len : 0;
    return runtime_stress_note_telemetry(context, bytes, 0, 0, 0.0);
}
