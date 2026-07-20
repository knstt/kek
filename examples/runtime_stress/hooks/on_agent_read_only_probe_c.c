#include "runtime_stress_support.h"

int OnAgentReadOnlyProbeC(KekHookContext* context) {
    return runtime_stress_readonly_probe(context, 0x94d049bb133111ebull);
}
