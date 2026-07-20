#include "examples/game/hooks/game_hook_support.h"

int move_declared_enemy(KekHookContext* context, size_t enemy_slot_id,
                        float speed, float strafe) {
    GameApp* app = (GameApp*)context->app_context;
    const Game_stateStateSlots* slots = game_state_get_slots_const(&app->state);
    const GameSession* session =
        game_state_session_const(context->state_store, slots);
    const Player* player = game_state_player_const(context->state_store, slots);
    const FrameClock* frame =
        game_state_frame_const(context->state_store, slots);
    const Enemy* enemy =
        game_state_enemy_slot_const(context->state_store, enemy_slot_id);
    if (!session || !player || !frame || !enemy || !enemy->active ||
        session->mode != GameMode_Playing) {
        return 1;
    }
    EnemyUpdate update = {*player, frame->dt, speed, strafe};
    return kek_state_store_update_fields(context->state_store, enemy_slot_id,
                                         enemy_update, &update,
                                         KEK_STATE_TYPE_ENEMY_FIELD_X |
                                             KEK_STATE_TYPE_ENEMY_FIELD_Y |
                                             KEK_STATE_TYPE_ENEMY_FIELD_VX |
                                             KEK_STATE_TYPE_ENEMY_FIELD_VY |
                                             KEK_STATE_TYPE_ENEMY_FIELD_FLASH);
}
