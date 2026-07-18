#include "examples/game/hooks/game_hook_support.h"

int MoveBossEnemy(KekHookContext* context) {
    return move_declared_enemy(context, game_state_boss_enemy_current_const,
                               game_state_update_boss_enemy, 54.0f, 0.28f);
}
