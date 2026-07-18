#include "examples/game/hooks/game_hook_support.h"

int MoveTankEnemy(KekHookContext* context) {
    return move_declared_enemy(context, game_state_tank_enemy_current_const,
                               game_state_update_tank_enemy, 70.0f, -0.10f);
}
