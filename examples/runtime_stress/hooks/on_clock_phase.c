#include "runtime_stress_support.h"

int OnClockPhase(KekHookContext* context) {
    return runtime_stress_fail_clock_phase(context);
}
