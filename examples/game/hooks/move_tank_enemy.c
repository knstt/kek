#include "examples/game/hooks/game_hook_support.h"

int MoveTankEnemy(KekHookContext* context) {
    GameApp* app = (GameApp*)context->app_context;
    return move_declared_enemy(context, game_state_tank_enemy_slot_id(&app->state),
                               70.0f, -0.10f);
}
