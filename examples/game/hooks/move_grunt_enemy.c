#include "examples/game/hooks/game_hook_support.h"

int MoveGruntEnemy(KekHookContext* context) {
    return move_declared_enemy(context, game_state_grunt_enemy_current_const,
                               game_state_update_grunt_enemy, 108.0f, 0.0f);
}
