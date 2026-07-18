#ifndef KEK_RUNTIME_DYNAMIC_HOOK_SMOKE_H
#define KEK_RUNTIME_DYNAMIC_HOOK_SMOKE_H

#include "runtime/hook.h"

typedef struct DynamicHookSmokeApp {
    int hook_value;
} DynamicHookSmokeApp;

int DynamicSmokeHook(KekHookContext* context);

#endif
