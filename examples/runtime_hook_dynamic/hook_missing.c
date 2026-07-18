#include "dynamic_hook_smoke.h"

int NotTheExpectedHook(KekHookContext* context) {
    (void)context;
    return 1;
}
