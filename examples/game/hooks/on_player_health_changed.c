#include "examples/game/game_app.h"
#include "examples/game/hooks/game_state_hooks.h"

int OnPlayerHealthChanged(KekHookContext* context) {
    GameApp* app = (GameApp*)context->app_context;
    OnPlayerHealthChangedAccess access = on_player_health_changed_access(
        context, game_state_get_slots_const(&app->state));
    const Player* player = on_player_health_changed_read_player(&access);
    const GameSession* session = on_player_health_changed_read_session(&access);
    if (!player || !session || player->health > 0 || session->mode == GameMode_GameOver) {
        return 1;
    }
    GameMode game_over = GameMode_GameOver;
    return on_player_health_changed_set_session_mode(&access, game_over);
}
