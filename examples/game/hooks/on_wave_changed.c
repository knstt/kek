#include "examples/game/game_app.h"
#include "examples/game/hooks/game_state_hooks.h"

int OnWaveChanged(KekHookContext* context) {
    GameApp* app = (GameApp*)context->app_context;
    const WaveDirector* wave = game_state_wave_current_const(&app->state);
    const GameSession* session = game_state_session_current_const(&app->state);
    if (!wave || !session) {
        return 1;
    }
    if (wave->wave >= 6 && wave->spawn_budget == 0 && wave->active_enemies == 0 &&
        session->mode == GameMode_Playing) {
        GameMode victory = GameMode_Victory;
        return game_state_set_session_mode(&app->state, victory);
    }
    return 1;
}
