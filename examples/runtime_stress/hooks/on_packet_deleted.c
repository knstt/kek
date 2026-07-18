#include "runtime_stress_support.h"

int OnPacketDeleted(KekHookContext* context) {
    return runtime_stress_note_audit(context, 1, 0, 0, 0);
}
