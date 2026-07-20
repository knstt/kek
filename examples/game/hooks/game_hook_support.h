#ifndef KEK_EXAMPLES_GAME_HOOK_SUPPORT_H
#define KEK_EXAMPLES_GAME_HOOK_SUPPORT_H

#include "examples/game/game_app.h"
#include "examples/game/hooks/game_state_hooks.h"

int move_declared_enemy(KekHookContext* context, size_t enemy_slot_id,
                        float speed, float strafe);

#endif
