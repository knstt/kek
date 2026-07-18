#include "examples/game/game_app.h"
#include "examples/game/hooks/game_state_hooks.h"

int OnFrameClock(KekHookContext* context) {
    GameApp* app = (GameApp*)context->app_context;
    const GameSession* session = game_state_session_current_const(&app->state);
    const WaveDirector* wave = game_state_wave_current_const(&app->state);
    if (!session || !wave || session->mode != GameMode_Playing) {
        return 1;
    }
    if (wave->spawn_budget == 0 && game_state_count_active_enemy(&app->state) == 0) {
        WaveDirector next = *wave;
        if (wave->wave >= 5) {
            next.wave = 6;
            next.spawn_budget = 0;
            next.spawn_timer = 0.0f;
        } else if (wave->wave == 0) {
            next.wave = 1;
            next.spawn_budget = 8;
            next.spawn_timer = 0.3f;
        } else {
            next.wave++;
            next.spawn_budget = 5 + next.wave * 3;
            next.spawn_timer = 0.3f;
            next.boss_spawned = false;
        }
        WaveFullUpdate update = {next};
        return game_state_update_wave(&app->state, wave_full_update, &update,
                                      KEK_EVENT_CHANGED_FIELDS_UNKNOWN);
    }
    return 1;
}
