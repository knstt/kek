#include "examples/game/game_app.h"
#include "examples/game/hooks/game_state_hooks.h"

int OnPlayerHealthChanged(KekHookContext* context) {
    GameApp* app = (GameApp*)context->app_context;
    const Player* player = game_state_player_current_const(&app->state);
    const GameSession* session = game_state_session_current_const(&app->state);
    if (!player || !session || player->health > 0 || session->mode == GameMode_GameOver) {
        return 1;
    }
    GameMode game_over = GameMode_GameOver;
    return game_state_set_session_mode(&app->state, game_over);
}
