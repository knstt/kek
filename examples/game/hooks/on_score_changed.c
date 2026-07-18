#include "examples/game/game_app.h"
#include "examples/game/hooks/game_state_hooks.h"

int OnScoreChanged(KekHookContext* context) {
    GameApp* app = (GameApp*)context->app_context;
    const GameSession* session = game_state_session_current_const(&app->state);
    if (!session || session->mode != GameMode_Playing) {
        return 1;
    }
    if (session->score >= session->next_upgrade_score) {
        ModeUpdate upgrade = {GameMode_Upgrade, session->next_upgrade_score + 600};
        return game_state_update_session(&app->state, set_mode_update, &upgrade,
                                         KEK_STATE_TYPE_GAME_SESSION_FIELD_MODE |
                                             KEK_STATE_TYPE_GAME_SESSION_FIELD_NEXT_UPGRADE_SCORE);
    }
    return 1;
}
