#include "examples/game/game_app.h"
#include "examples/game/hooks/game_state_hooks.h"

int OnScoreChanged(KekHookContext* context) {
    GameApp* app = (GameApp*)context->app_context;
    OnScoreChangedAccess access =
        on_score_changed_access(context, game_state_get_slots_const(&app->state));
    const GameSession* session = on_score_changed_read_session(&access);
    if (!session || session->mode != GameMode_Playing) {
        return 1;
    }
    if (session->score >= session->next_upgrade_score) {
        ModeUpdate upgrade = {GameMode_Upgrade, session->next_upgrade_score + 600};
        return on_score_changed_update_session(
            &access, set_mode_update, &upgrade,
            KEK_STATE_TYPE_GAME_SESSION_FIELD_MODE |
                KEK_STATE_TYPE_GAME_SESSION_FIELD_NEXT_UPGRADE_SCORE);
    }
    return 1;
}
