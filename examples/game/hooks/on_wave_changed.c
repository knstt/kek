#include "examples/game/game_app.h"
#include "examples/game/hooks/game_state_hooks.h"

int OnWaveChanged(KekHookContext* context) {
    GameApp* app = (GameApp*)context->app_context;
    OnWaveChangedAccess access =
        on_wave_changed_access(context, game_state_get_slots_const(&app->state));
    const WaveDirector* wave = on_wave_changed_read_wave(&access);
    const GameSession* session = on_wave_changed_read_session(&access);
    if (!wave || !session) {
        return 1;
    }
    if (wave->wave >= 6 && wave->spawn_budget == 0 && wave->active_enemies == 0 &&
        session->mode == GameMode_Playing) {
        GameMode victory = GameMode_Victory;
        return on_wave_changed_set_session_mode(&access, victory);
    }
    return 1;
}
