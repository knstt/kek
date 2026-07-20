#include "runtime_stress_support.h"

int OnAgentReadOnlyProbeD(KekHookContext* context) {
    return runtime_stress_readonly_probe(context, 0xd6e8feb86659fd93ull);
}
