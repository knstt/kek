#include "dynamic_hook_smoke.h"

int DynamicSmokeHook(KekHookContext* context) {
    DynamicHookSmokeApp* app = (DynamicHookSmokeApp*)context->app_context;
    app->hook_value += 10;
    return 1;
}
