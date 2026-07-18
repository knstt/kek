#include "examples/game/hooks/game_hook_support.h"

int move_declared_enemy(KekHookContext* context, DeclaredEnemyReadFn read_enemy,
                        DeclaredEnemyUpdateFn update_enemy, float speed, float strafe) {
    GameApp* app = (GameApp*)context->app_context;
    const GameSession* session = game_state_session_current_const(&app->state);
    const Player* player = game_state_player_current_const(&app->state);
    const FrameClock* frame = game_state_frame_current_const(&app->state);
    const Enemy* enemy = read_enemy(&app->state);
    if (!session || !player || !frame || !enemy || !enemy->active ||
        session->mode != GameMode_Playing) {
        return 1;
    }
    EnemyUpdate update = {*player, frame->dt, speed, strafe};
    return update_enemy(&app->state, enemy_update, &update,
                        KEK_STATE_TYPE_ENEMY_FIELD_X |
                            KEK_STATE_TYPE_ENEMY_FIELD_Y |
                            KEK_STATE_TYPE_ENEMY_FIELD_VX |
                            KEK_STATE_TYPE_ENEMY_FIELD_VY |
                            KEK_STATE_TYPE_ENEMY_FIELD_FLASH);
}
