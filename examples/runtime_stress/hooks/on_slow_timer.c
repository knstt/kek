#include "runtime_stress_support.h"

int OnSlowTimer(KekHookContext* context) {
    return runtime_stress_note_audit(context, 0, 0, 0, 0);
}
