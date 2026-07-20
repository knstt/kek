#include "examples/game/hooks/game_hook_support.h"

int MoveGruntEnemy(KekHookContext* context) {
    GameApp* app = (GameApp*)context->app_context;
    return move_declared_enemy(context, game_state_grunt_enemy_slot_id(&app->state),
                               108.0f, 0.0f);
}
