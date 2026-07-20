#include "examples/game/hooks/game_hook_support.h"

int MoveRunnerEnemy(KekHookContext* context) {
    GameApp* app = (GameApp*)context->app_context;
    return move_declared_enemy(context, game_state_runner_enemy_slot_id(&app->state),
                               182.0f, 0.18f);
}
