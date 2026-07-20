#include "runtime_stress_support.h"

int OnAgentReadOnlyProbeA(KekHookContext* context) {
    return runtime_stress_readonly_probe(context, 0x9e3779b97f4a7c15ull);
}
