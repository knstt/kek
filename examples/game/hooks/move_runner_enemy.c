#include "examples/game/hooks/game_hook_support.h"

int MoveRunnerEnemy(KekHookContext* context) {
    return move_declared_enemy(context, game_state_runner_enemy_current_const,
                               game_state_update_runner_enemy, 182.0f, 0.18f);
}
