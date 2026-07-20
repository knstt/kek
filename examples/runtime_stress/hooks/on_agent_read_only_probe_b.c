#include "runtime_stress_support.h"

int OnAgentReadOnlyProbeB(KekHookContext* context) {
    return runtime_stress_readonly_probe(context, 0xbf58476d1ce4e5b9ull);
}
