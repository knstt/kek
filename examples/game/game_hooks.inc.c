int OnFrameClock(KekHookContext* context) {
    GameApp* app = (GameApp*)context->app_context;
    const GameSession* session = game_state_session_const(context->state_store, &app->binding.slots);
    const WaveDirector* wave = game_state_wave_const(context->state_store, &app->binding.slots);
    if (!session || !wave || session->mode != GameMode_Playing) {
        return 1;
    }
    if (wave->spawn_budget == 0 &&
        count_active_enemies(context->state_store) == 0) {
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
        return kek_state_store_update_fields(context->state_store, app->binding.slots.wave,
                                             wave_full_update, &update,
                                             KEK_EVENT_CHANGED_FIELDS_UNKNOWN);
    }
    return 1;
}

static int move_declared_enemy(KekHookContext* context, size_t slot, float speed, float strafe) {
    GameApp* app = (GameApp*)context->app_context;
    const GameSession* session = game_state_session_const(context->state_store, &app->binding.slots);
    const Player* player = game_state_player_const(context->state_store, &app->binding.slots);
    const FrameClock* frame = game_state_frame_const(context->state_store, &app->binding.slots);
    const Enemy* enemy = game_state_enemy_slot_const(context->state_store, slot);
    if (!session || !player || !frame || !enemy || !enemy->active || session->mode != GameMode_Playing) {
        return 1;
    }
    EnemyUpdate update = {*player, frame->dt, speed, strafe};
    return kek_state_store_update_fields(context->state_store, slot, enemy_update, &update,
                                         KEK_STATE_TYPE_ENEMY_FIELD_X |
                                             KEK_STATE_TYPE_ENEMY_FIELD_Y |
                                             KEK_STATE_TYPE_ENEMY_FIELD_VX |
                                             KEK_STATE_TYPE_ENEMY_FIELD_VY |
                                             KEK_STATE_TYPE_ENEMY_FIELD_FLASH);
}

int MoveGruntEnemy(KekHookContext* context) {
    GameApp* app = (GameApp*)context->app_context;
    return move_declared_enemy(context, app->binding.slots.grunt_enemy, 108.0f, 0.0f);
}

int MoveRunnerEnemy(KekHookContext* context) {
    GameApp* app = (GameApp*)context->app_context;
    return move_declared_enemy(context, app->binding.slots.runner_enemy, 182.0f, 0.18f);
}

int MoveTankEnemy(KekHookContext* context) {
    GameApp* app = (GameApp*)context->app_context;
    return move_declared_enemy(context, app->binding.slots.tank_enemy, 70.0f, -0.10f);
}

int MoveBossEnemy(KekHookContext* context) {
    GameApp* app = (GameApp*)context->app_context;
    return move_declared_enemy(context, app->binding.slots.boss_enemy, 54.0f, 0.28f);
}

int OnPlayerHealthChanged(KekHookContext* context) {
    GameApp* app = (GameApp*)context->app_context;
    const Player* player = game_state_player_const(context->state_store, &app->binding.slots);
    const GameSession* session = game_state_session_const(context->state_store, &app->binding.slots);
    if (!player || !session || player->health > 0 || session->mode == GameMode_GameOver) {
        return 1;
    }
    GameMode game_over = GameMode_GameOver;
    return kek_state_store_update_fields(context->state_store, app->binding.slots.session,
                                         set_mode_only_update, &game_over,
                                         KEK_STATE_TYPE_GAME_SESSION_FIELD_MODE);
}

int OnWaveChanged(KekHookContext* context) {
    GameApp* app = (GameApp*)context->app_context;
    const WaveDirector* wave = game_state_wave_const(context->state_store, &app->binding.slots);
    const GameSession* session = game_state_session_const(context->state_store, &app->binding.slots);
    if (!wave || !session) {
        return 1;
    }
    if (wave->wave >= 6 && wave->spawn_budget == 0 && wave->active_enemies == 0 &&
        session->mode == GameMode_Playing) {
        GameMode victory = GameMode_Victory;
        return kek_state_store_update_fields(context->state_store, app->binding.slots.session,
                                             set_mode_only_update, &victory,
                                             KEK_STATE_TYPE_GAME_SESSION_FIELD_MODE);
    }
    return 1;
}

int OnScoreChanged(KekHookContext* context) {
    GameApp* app = (GameApp*)context->app_context;
    const GameSession* session = game_state_session_const(context->state_store, &app->binding.slots);
    if (!session || session->mode != GameMode_Playing) {
        return 1;
    }
    if (session->score >= session->next_upgrade_score) {
        ModeUpdate upgrade = {GameMode_Upgrade, session->next_upgrade_score + 600};
        return kek_state_store_update_fields(context->state_store, app->binding.slots.session,
                                             set_mode_update, &upgrade,
                                             KEK_STATE_TYPE_GAME_SESSION_FIELD_MODE |
                                                 KEK_STATE_TYPE_GAME_SESSION_FIELD_NEXT_UPGRADE_SCORE);
    }
    return 1;
}
