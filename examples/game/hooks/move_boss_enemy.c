#include "examples/game/hooks/game_hook_support.h"

int MoveBossEnemy(KekHookContext* context) {
    GameApp* app = (GameApp*)context->app_context;
    return move_declared_enemy(context, game_state_boss_enemy_slot_id(&app->state),
                               54.0f, 0.28f);
}
