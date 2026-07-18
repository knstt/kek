int OnFrameClock(KekHookContext* context) {
    GameApp* app = (GameApp*)context->app_context;
    const GameSession* session = game_state_session_current_const(&app->state);
    const WaveDirector* wave = game_state_wave_current_const(&app->state);
    if (!session || !wave || session->mode != GameMode_Playing) {
        return 1;
    }
    if (wave->spawn_budget == 0 &&
        game_state_count_active_enemy(&app->state) == 0) {
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

typedef const Enemy* (*DeclaredEnemyReadFn)(const Game_stateRuntime* runtime);
typedef int (*DeclaredEnemyUpdateFn)(Game_stateRuntime* runtime,
                                     KekStateStorageUpdateFn update,
                                     void* context,
                                     uint64_t changed_fields);

static int move_declared_enemy(KekHookContext* context, DeclaredEnemyReadFn read_enemy,
                               DeclaredEnemyUpdateFn update_enemy, float speed, float strafe) {
    GameApp* app = (GameApp*)context->app_context;
    const GameSession* session = game_state_session_current_const(&app->state);
    const Player* player = game_state_player_current_const(&app->state);
    const FrameClock* frame = game_state_frame_current_const(&app->state);
    const Enemy* enemy = read_enemy(&app->state);
    if (!session || !player || !frame || !enemy || !enemy->active || session->mode != GameMode_Playing) {
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

int MoveGruntEnemy(KekHookContext* context) {
    return move_declared_enemy(context, game_state_grunt_enemy_current_const,
                               game_state_update_grunt_enemy, 108.0f, 0.0f);
}

int MoveRunnerEnemy(KekHookContext* context) {
    return move_declared_enemy(context, game_state_runner_enemy_current_const,
                               game_state_update_runner_enemy, 182.0f, 0.18f);
}

int MoveTankEnemy(KekHookContext* context) {
    return move_declared_enemy(context, game_state_tank_enemy_current_const,
                               game_state_update_tank_enemy, 70.0f, -0.10f);
}

int MoveBossEnemy(KekHookContext* context) {
    return move_declared_enemy(context, game_state_boss_enemy_current_const,
                               game_state_update_boss_enemy, 54.0f, 0.28f);
}

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
