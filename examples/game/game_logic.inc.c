static void set_mode_update(void* draft, void* context) {
    GameSession* session = (GameSession*)draft;
    const ModeUpdate* update = (const ModeUpdate*)context;
    session->mode = update->mode;
    session->next_upgrade_score = update->next_upgrade_score;
}

static void session_frame_update(void* draft, void* context) {
    GameSession* session = (GameSession*)draft;
    const SessionFrameUpdate* update = (const SessionFrameUpdate*)context;
    if (update->toggle_debug) {
        session->debug = !session->debug;
    }
    session->time_alive += update->dt;
    session->combo_timer = clampf(session->combo_timer - update->dt, 0.0f, 5.0f);
    if (session->combo_timer <= 0.0f) {
        session->combo = 0;
    }
    session->shake = clampf(session->shake - update->dt * 1.8f, 0.0f, 1.0f);
}

static void score_update(void* draft, void* context) {
    GameSession* session = (GameSession*)draft;
    const ScoreUpdate* update = (const ScoreUpdate*)context;
    session->score += update->score_delta;
    session->combo = (int32_t)clampf((float)(session->combo + update->combo_delta), 0.0f, 99.0f);
    if (update->combo_delta > 0) {
        session->combo_timer = 3.5f;
    }
}

static void add_shake_update(void* draft, void* context) {
    GameSession* session = (GameSession*)draft;
    const float* amount = (const float*)context;
    session->shake = clampf(session->shake + *amount, 0.0f, 1.0f);
}

static void input_update(void* draft, void* context) {
    InputIntent* input = (InputIntent*)draft;
    const InputIntent* next = (const InputIntent*)context;
    *input = *next;
}

static void player_start_update(void* draft, void* context) {
    (void)context;
    Player* player = (Player*)draft;
    *player = Player_default();
}

static void session_start_update(void* draft, void* context) {
    (void)context;
    GameSession* session = (GameSession*)draft;
    *session = GameSession_playing();
}

static void wave_start_update(void* draft, void* context) {
    (void)context;
    WaveDirector* wave = (WaveDirector*)draft;
    *wave = WaveDirector_default();
    wave->wave = 1;
    wave->spawn_budget = 6;
    wave->spawn_timer = 0.2f;
}

static void camera_start_update(void* draft, void* context) {
    (void)context;
    CameraRig* camera = (CameraRig*)draft;
    *camera = CameraRig_default();
}

static void frame_clock_reset_update(void* draft, void* context) {
    (void)context;
    FrameClock* frame = (FrameClock*)draft;
    *frame = FrameClock_default();
}

static void frame_clock_update(void* draft, void* context) {
    FrameClock* frame = (FrameClock*)draft;
    const FrameClockUpdate* update = (const FrameClockUpdate*)context;
    frame->tick = update->tick;
    frame->dt = update->dt;
}

static void player_frame_update(void* draft, void* context) {
    Player* player = (Player*)draft;
    const PlayerFrameUpdate* update = (const PlayerFrameUpdate*)context;
    float move_x = update->input.move_x;
    float move_y = update->input.move_y;
    normalize_or(&move_x, &move_y, 0.0f, 0.0f);

    float speed = 285.0f + (float)(player->level - 1) * 14.0f;
    if (update->input.dash && player->dash_cooldown <= 0.0f) {
        player->vx = move_x * 850.0f;
        player->vy = move_y * 850.0f;
        player->dash_cooldown = 1.25f;
        player->invulnerable_timer = 0.35f;
    } else {
        player->vx = move_x * speed;
        player->vy = move_y * speed;
    }

    player->x = clampf(player->x + player->vx * update->dt, -ARENA_HALF_WIDTH, ARENA_HALF_WIDTH);
    player->y = clampf(player->y + player->vy * update->dt, -ARENA_HALF_HEIGHT, ARENA_HALF_HEIGHT);
    player->fire_cooldown = clampf(player->fire_cooldown - update->dt, 0.0f, 2.0f);
    player->dash_cooldown = clampf(player->dash_cooldown - update->dt, 0.0f, 4.0f);
    player->rapid_timer = clampf(player->rapid_timer - update->dt, 0.0f, 12.0f);
    player->invulnerable_timer = clampf(player->invulnerable_timer - update->dt, 0.0f, 3.0f);
}

static void player_damage_update(void* draft, void* context) {
    Player* player = (Player*)draft;
    const PlayerDamageUpdate* update = (const PlayerDamageUpdate*)context;
    int remaining = update->damage;
    if (player->shield > 0) {
        int absorbed = player->shield < remaining ? player->shield : remaining;
        player->shield -= absorbed;
        remaining -= absorbed;
    }
    player->health -= remaining;
    if (player->health < 0) {
        player->health = 0;
    }
    player->invulnerable_timer = 0.65f;
}

static void player_reward_update(void* draft, void* context) {
    Player* player = (Player*)draft;
    const PlayerRewardUpdate* update = (const PlayerRewardUpdate*)context;
    player->xp += update->xp;
    while (player->level < 10 && player->xp >= player->level * 90) {
        player->xp -= player->level * 90;
        player->level++;
        player->max_health = player->max_health < 150 ? player->max_health + 10 : 150;
        player->health = player->max_health;
    }
}

static void player_pickup_update(void* draft, void* context) {
    Player* player = (Player*)draft;
    const PlayerPickupUpdate* update = (const PlayerPickupUpdate*)context;
    if (update->kind == PickupKind_Health) {
        player->health += update->value;
        if (player->health > player->max_health) {
            player->health = player->max_health;
        }
    } else if (update->kind == PickupKind_RapidFire) {
        player->rapid_timer = 8.0f;
    } else if (update->kind == PickupKind_Shield) {
        player->shield += update->value;
        if (player->shield > 100) {
            player->shield = 100;
        }
    }
}

static void camera_frame_update(void* draft, void* context) {
    CameraRig* camera = (CameraRig*)draft;
    const CameraFrameUpdate* update = (const CameraFrameUpdate*)context;
    camera->x += (update->player.x - camera->x) * clampf(update->dt * 8.0f, 0.0f, 1.0f);
    camera->y += (update->player.y - camera->y) * clampf(update->dt * 8.0f, 0.0f, 1.0f);
    camera->zoom = 1.0f + (float)(update->player.level - 1) * 0.015f;
}

static void wave_runtime_update(void* draft, void* context) {
    WaveDirector* wave = (WaveDirector*)draft;
    const WaveRuntimeUpdate* update = (const WaveRuntimeUpdate*)context;
    wave->active_enemies = update->active_enemies;
    wave->spawn_timer = clampf(wave->spawn_timer - update->dt, 0.0f, 10.0f);
}

static void wave_full_update(void* draft, void* context) {
    WaveDirector* wave = (WaveDirector*)draft;
    const WaveFullUpdate* update = (const WaveFullUpdate*)context;
    *wave = update->wave;
}

static void wave_spawn_update(void* draft, void* context) {
    WaveDirector* wave = (WaveDirector*)draft;
    const WaveSpawnUpdate* update = (const WaveSpawnUpdate*)context;
    if (wave->spawn_budget > 0) {
        wave->spawn_budget--;
    }
    wave->spawn_timer = update->next_spawn_timer;
    wave->boss_spawned = update->boss_spawned;
}

static void projectile_update(void* draft, void* context) {
    Projectile* projectile = (Projectile*)draft;
    const ProjectileUpdate* update = (const ProjectileUpdate*)context;
    projectile->x += projectile->vx * update->dt;
    projectile->y += projectile->vy * update->dt;
    projectile->life = clampf(projectile->life - update->dt, 0.0f, 5.0f);
}

static void projectile_pierce_update(void* draft, void* context) {
    Projectile* projectile = (Projectile*)draft;
    (void)context;
    if (projectile->pierce > 0) {
        projectile->pierce--;
    }
}

static void enemy_update(void* draft, void* context) {
    Enemy* enemy = (Enemy*)draft;
    const EnemyUpdate* update = (const EnemyUpdate*)context;
    if (!enemy->active) {
        return;
    }
    float dir_x = update->player.x - enemy->x;
    float dir_y = update->player.y - enemy->y;
    normalize_or(&dir_x, &dir_y, 0.0f, 0.0f);

    float move_x = dir_x - dir_y * update->strafe;
    float move_y = dir_y + dir_x * update->strafe;
    normalize_or(&move_x, &move_y, dir_x, dir_y);

    enemy->vx = move_x * update->speed;
    enemy->vy = move_y * update->speed;
    enemy->x = clampf(enemy->x + enemy->vx * update->dt, -ARENA_HALF_WIDTH - 160.0f, ARENA_HALF_WIDTH + 160.0f);
    enemy->y = clampf(enemy->y + enemy->vy * update->dt, -ARENA_HALF_HEIGHT - 160.0f, ARENA_HALF_HEIGHT + 160.0f);
    enemy->flash = clampf(enemy->flash - update->dt * 4.5f, 0.0f, 1.0f);
}

static float enemy_speed_for_kind(EnemyKind kind) {
    if (kind == EnemyKind_Runner) {
        return 172.0f;
    }
    if (kind == EnemyKind_Tank) {
        return 76.0f;
    }
    if (kind == EnemyKind_Boss) {
        return 58.0f;
    }
    return 108.0f;
}

static void enemy_damage_update(void* draft, void* context) {
    Enemy* enemy = (Enemy*)draft;
    const EnemyDamageUpdate* update = (const EnemyDamageUpdate*)context;
    enemy->health -= update->damage;
    if (enemy->health < 0) {
        enemy->health = 0;
    }
    enemy->flash = 1.0f;
    if (enemy->health <= 0) {
        enemy->active = false;
    }
}

static void enemy_reset_update(void* draft, void* context) {
    Enemy* enemy = (Enemy*)draft;
    const EnemyResetUpdate* update = (const EnemyResetUpdate*)context;
    *enemy = update->enemy;
}

static void pickup_update(void* draft, void* context) {
    Pickup* pickup = (Pickup*)draft;
    const PickupUpdate* update = (const PickupUpdate*)context;
    pickup->life = clampf(pickup->life - update->dt, 0.0f, 30.0f);
    pickup->pulse = clampf(pickup->pulse + update->dt * 8.0f, 0.0f, 1000000.0f);
}

static void hud_update(void* draft, void* context) {
    HudMessage* hud = (HudMessage*)draft;
    const HudUpdate* update = (const HudUpdate*)context;
    hud->y -= 38.0f * update->dt;
    hud->life = clampf(hud->life - update->dt, 0.0f, 3.0f);
}

static void clear_dynamic_entities(GameApp* app) {
    for (size_t slot = game_state_first_enemy(&app->state); slot != KEK_STATE_INVALID_ID;) {
        size_t next = game_state_next_enemy(&app->state, slot);
        if (!game_state_is_declared_enemy_slot(&app->state, slot)) {
            (void)game_state_delete_enemy(&app->state, slot);
        }
        slot = next;
    }
    for (size_t slot = game_state_first_projectile(&app->state); slot != KEK_STATE_INVALID_ID;) {
        size_t next = game_state_next_projectile(&app->state, slot);
        (void)game_state_delete_projectile(&app->state, slot);
        slot = next;
    }
    for (size_t slot = game_state_first_pickup(&app->state); slot != KEK_STATE_INVALID_ID;) {
        size_t next = game_state_next_pickup(&app->state, slot);
        (void)game_state_delete_pickup(&app->state, slot);
        slot = next;
    }
    for (size_t slot = game_state_first_hud_message(&app->state); slot != KEK_STATE_INVALID_ID;) {
        size_t next = game_state_next_hud_message(&app->state, slot);
        (void)game_state_delete_hud_message(&app->state, slot);
        slot = next;
    }
}

static void reset_declared_enemies(GameApp* app) {
    Enemy grunt = Enemy_default();
    grunt.kind = EnemyKind_Grunt;
    grunt.x = -440.0f;
    grunt.y = -220.0f;

    Enemy runner = Enemy_runner();
    runner.x = 430.0f;
    runner.y = -190.0f;

    Enemy tank = Enemy_tank();
    tank.x = -520.0f;
    tank.y = 240.0f;

    Enemy boss = Enemy_boss();
    boss.x = 540.0f;
    boss.y = 250.0f;

    EnemyResetUpdate grunt_update = {grunt};
    EnemyResetUpdate runner_update = {runner};
    EnemyResetUpdate tank_update = {tank};
    EnemyResetUpdate boss_update = {boss};
    Game_stateUpdateItem updates[] = {
        game_state_grunt_enemy_update_item(&app->state, enemy_reset_update, &grunt_update, KEK_EVENT_CHANGED_FIELDS_UNKNOWN),
        game_state_runner_enemy_update_item(&app->state, enemy_reset_update, &runner_update, KEK_EVENT_CHANGED_FIELDS_UNKNOWN),
        game_state_tank_enemy_update_item(&app->state, enemy_reset_update, &tank_update, KEK_EVENT_CHANGED_FIELDS_UNKNOWN),
        game_state_boss_enemy_update_item(&app->state, enemy_reset_update, &boss_update, KEK_EVENT_CHANGED_FIELDS_UNKNOWN),
    };
    (void)game_state_update_many(&app->state, updates, sizeof(updates) / sizeof(updates[0]));
}

static void add_hud_message(GameApp* app, HudMessageKind kind, float x, float y, int value) {
    HudMessage message = HudMessage_default();
    message.kind = kind;
    message.x = x;
    message.y = y;
    message.value = value;
    message.life = 1.3f;
    (void)game_state_create_hud_message_with(&app->state, &message);
}

static void add_pickup(GameApp* app, float x, float y, int score_value) {
    int roll = GetRandomValue(0, 99);
    Pickup pickup = Pickup_default();
    pickup.x = x;
    pickup.y = y;
    if (roll < 16) {
        pickup.kind = PickupKind_Health;
        pickup.value = 18;
    } else if (roll < 26) {
        pickup.kind = PickupKind_Shield;
        pickup.value = 15;
    } else if (roll < 34) {
        pickup.kind = PickupKind_RapidFire;
        pickup.value = 1;
    } else {
        pickup.kind = PickupKind_Score;
        pickup.value = score_value;
    }
    (void)game_state_create_pickup_with(&app->state, &pickup);
}

static void start_game(GameApp* app) {
    clear_dynamic_entities(app);
    Game_stateUpdateItem updates[] = {
        game_state_frame_update_item(&app->state, frame_clock_reset_update, NULL, KEK_EVENT_CHANGED_FIELDS_UNKNOWN),
        game_state_session_update_item(&app->state, session_start_update, NULL, KEK_EVENT_CHANGED_FIELDS_UNKNOWN),
        game_state_player_update_item(&app->state, player_start_update, NULL, KEK_EVENT_CHANGED_FIELDS_UNKNOWN),
        game_state_wave_update_item(&app->state, wave_start_update, NULL, KEK_EVENT_CHANGED_FIELDS_UNKNOWN),
        game_state_camera_update_item(&app->state, camera_start_update, NULL, KEK_EVENT_CHANGED_FIELDS_UNKNOWN),
    };
    (void)game_state_update_many(&app->state, updates, sizeof(updates) / sizeof(updates[0]));
    reset_declared_enemies(app);
    add_hud_message(app, HudMessageKind_Info, 0.0f, -60.0f, HUD_VALUE_WARNING);
    (void)game_state_dispatch(&app->state);
}

static InputIntent collect_input(GameApp* app) {
    const InputIntent* previous_input = game_state_input_current_const(&app->state);
    const Player* player = game_state_player_current_const(&app->state);
    InputIntent input = InputIntent_default();
    input.move_x = (float)IsKeyDown(KEY_D) - (float)IsKeyDown(KEY_A);
    input.move_y = (float)IsKeyDown(KEY_S) - (float)IsKeyDown(KEY_W);
    normalize_or(&input.move_x, &input.move_y, 0.0f, 0.0f);

    float aim_x = previous_input ? previous_input->aim_x : 1.0f;
    float aim_y = previous_input ? previous_input->aim_y : 0.0f;
    float aim_step = 0.17f;
    bool keyboard_aim = IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_UP);
    if (keyboard_aim) {
        aim_x += ((float)IsKeyDown(KEY_RIGHT) - (float)IsKeyDown(KEY_LEFT)) * aim_step;
        aim_y += ((float)IsKeyDown(KEY_DOWN) - (float)IsKeyDown(KEY_UP)) * aim_step;
    }
    input.aim_x = aim_x;
    input.aim_y = aim_y;
    normalize_or(&input.aim_x, &input.aim_y, 1.0f, 0.0f);
    if (player && !keyboard_aim) {
        Vector2 mouse_world = GetScreenToWorld2D(GetMousePosition(), camera_for(app));
        input.aim_x = mouse_world.x - player->x;
        input.aim_y = mouse_world.y - player->y;
        normalize_or(&input.aim_x, &input.aim_y, aim_x, aim_y);
    }
    input.shoot = IsKeyDown(KEY_SPACE) || IsMouseButtonDown(MOUSE_BUTTON_LEFT);
    input.dash = IsKeyPressed(KEY_LEFT_SHIFT) || IsMouseButtonPressed(MOUSE_BUTTON_RIGHT);
    input.pause = IsKeyPressed(KEY_P) || IsKeyPressed(KEY_ESCAPE);
    input.confirm = IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE);
    return input;
}

static void spawn_projectile(GameApp* app, const Player* player, const InputIntent* input) {
    float aim_x = input->aim_x;
    float aim_y = input->aim_y;
    normalize_or(&aim_x, &aim_y, 1.0f, 0.0f);
    Projectile projectile = Projectile_default();
    projectile.x = player->x + aim_x * (PLAYER_RADIUS + 10.0f);
    projectile.y = player->y + aim_y * (PLAYER_RADIUS + 10.0f);
    projectile.vx = aim_x * 760.0f;
    projectile.vy = aim_y * 760.0f;
    projectile.damage = 16 + player->level * 3;
    projectile.pierce = player->rapid_timer > 0.0f ? 2 : 1;
    (void)game_state_create_projectile_with(&app->state, &projectile);

    float cooldown = player->rapid_timer > 0.0f ? 0.095f : 0.18f;
    (void)game_state_set_player_fire_cooldown(&app->state, cooldown);
}

static Enemy enemy_for_wave(int wave, bool boss) {
    Enemy enemy = Enemy_default();
    if (boss) {
        enemy = Enemy_boss();
    } else if (wave >= 3 && GetRandomValue(0, 99) < 24) {
        enemy = Enemy_tank();
    } else if (GetRandomValue(0, 99) < 42) {
        enemy = Enemy_runner();
    }
    enemy.health += wave * 5;
    enemy.damage += wave / 2;
    return enemy;
}

static void spawn_enemy(GameApp* app) {
    const WaveDirector* wave = game_state_wave_current_const(&app->state);
    if (!wave || wave->spawn_budget <= 0) {
        return;
    }
    bool boss = wave->wave >= 5 && !wave->boss_spawned && wave->spawn_budget <= 1;
    Enemy enemy = enemy_for_wave(wave->wave, boss);
    int side = GetRandomValue(0, 3);
    if (side == 0) {
        enemy.x = -ARENA_HALF_WIDTH - 80.0f;
        enemy.y = (float)GetRandomValue((int)-ARENA_HALF_HEIGHT, (int)ARENA_HALF_HEIGHT);
    } else if (side == 1) {
        enemy.x = ARENA_HALF_WIDTH + 80.0f;
        enemy.y = (float)GetRandomValue((int)-ARENA_HALF_HEIGHT, (int)ARENA_HALF_HEIGHT);
    } else if (side == 2) {
        enemy.x = (float)GetRandomValue((int)-ARENA_HALF_WIDTH, (int)ARENA_HALF_WIDTH);
        enemy.y = -ARENA_HALF_HEIGHT - 80.0f;
    } else {
        enemy.x = (float)GetRandomValue((int)-ARENA_HALF_WIDTH, (int)ARENA_HALF_WIDTH);
        enemy.y = ARENA_HALF_HEIGHT + 80.0f;
    }
    (void)game_state_create_enemy_with(&app->state, &enemy);
    WaveSpawnUpdate update = {
        clampf(0.72f - (float)wave->wave * 0.055f, 0.22f, 0.72f),
        boss ? true : wave->boss_spawned,
    };
    (void)game_state_update_wave(&app->state, wave_spawn_update, &update,
                                 KEK_STATE_TYPE_WAVE_DIRECTOR_FIELD_SPAWN_BUDGET |
                                     KEK_STATE_TYPE_WAVE_DIRECTOR_FIELD_SPAWN_TIMER |
                                     KEK_STATE_TYPE_WAVE_DIRECTOR_FIELD_BOSS_SPAWNED);
}

static void advance_frame_clock(GameApp* app, float dt) {
    const FrameClock* frame = game_state_frame_current_const(&app->state);
    if (!frame) {
        return;
    }
    FrameClockUpdate update = {frame->tick + 1, dt};
    (void)game_state_update_frame(&app->state, frame_clock_update, &update,
                                  KEK_STATE_TYPE_FRAME_CLOCK_FIELD_TICK |
                                      KEK_STATE_TYPE_FRAME_CLOCK_FIELD_DT);
}

static void update_projectiles(GameApp* app, float dt) {
    for (size_t projectile_slot = game_state_first_projectile(&app->state); projectile_slot != KEK_STATE_INVALID_ID;) {
        size_t next_projectile = game_state_next_projectile(&app->state, projectile_slot);
        const Projectile* projectile = game_state_projectile_at_const(&app->state, projectile_slot);
        if (!projectile) {
            projectile_slot = next_projectile;
            continue;
        }

        bool remove_projectile = projectile->life <= 0.0f || projectile->pierce <= 0 ||
                                 fabsf(projectile->x) > ARENA_HALF_WIDTH + 220.0f ||
                                 fabsf(projectile->y) > ARENA_HALF_HEIGHT + 220.0f;
        if (!remove_projectile) {
            ProjectileUpdate update = {dt};
            (void)game_state_update_projectile_slot(&app->state, projectile_slot, projectile_update, &update,
                                                    KEK_STATE_TYPE_PROJECTILE_FIELD_X |
                                                        KEK_STATE_TYPE_PROJECTILE_FIELD_Y |
                                                        KEK_STATE_TYPE_PROJECTILE_FIELD_LIFE);
            projectile = game_state_projectile_at_const(&app->state, projectile_slot);
        }

        if (!remove_projectile && projectile) {
            for (size_t enemy_slot = game_state_first_enemy(&app->state); enemy_slot != KEK_STATE_INVALID_ID;) {
                size_t next_enemy = game_state_next_enemy(&app->state, enemy_slot);
                const Enemy* enemy = game_state_enemy_at_const(&app->state, enemy_slot);
                projectile = game_state_projectile_at_const(&app->state, projectile_slot);
                if (!enemy || !enemy->active || !projectile) {
                    enemy_slot = next_enemy;
                    continue;
                }
                float distance = vec_len(projectile->x - enemy->x, projectile->y - enemy->y);
                if (distance <= enemy->radius + PROJECTILE_RADIUS) {
                    EnemyDamageUpdate hit = {projectile->damage};
                    (void)game_state_update_enemy_slot(&app->state, enemy_slot, enemy_damage_update, &hit,
                                                       KEK_STATE_TYPE_ENEMY_FIELD_HEALTH |
                                                           KEK_STATE_TYPE_ENEMY_FIELD_FLASH);
                    (void)game_state_update_projectile_slot(&app->state, projectile_slot, projectile_pierce_update,
                                                            NULL, KEK_STATE_TYPE_PROJECTILE_FIELD_PIERCE);
                    enemy = game_state_enemy_at_const(&app->state, enemy_slot);
                    projectile = game_state_projectile_at_const(&app->state, projectile_slot);
                    if (enemy && enemy->health <= 0) {
                        int reward = enemy->kind == EnemyKind_Boss ? 1000 : 35 + (int)enemy->radius;
                        ScoreUpdate score = {reward, 1};
                        PlayerRewardUpdate xp = {enemy->kind == EnemyKind_Boss ? 140 : 16};
                        (void)game_state_update_session(&app->state, score_update, &score,
                                                        KEK_STATE_TYPE_GAME_SESSION_FIELD_SCORE |
                                                            KEK_STATE_TYPE_GAME_SESSION_FIELD_COMBO |
                                                            KEK_STATE_TYPE_GAME_SESSION_FIELD_COMBO_TIMER);
                        (void)game_state_update_player(&app->state, player_reward_update, &xp,
                                                       KEK_STATE_TYPE_PLAYER_FIELD_XP |
                                                           KEK_STATE_TYPE_PLAYER_FIELD_LEVEL |
                                                           KEK_STATE_TYPE_PLAYER_FIELD_HEALTH |
                                                           KEK_STATE_TYPE_PLAYER_FIELD_MAX_HEALTH);
                        add_pickup(app, enemy->x, enemy->y, reward / 3);
                        add_hud_message(app, HudMessageKind_Reward, enemy->x, enemy->y, reward);
                        if (!game_state_is_declared_enemy_slot(&app->state, enemy_slot)) {
                            (void)game_state_delete_enemy(&app->state, enemy_slot);
                        }
                    }
                    if (!projectile || projectile->pierce <= 0) {
                        remove_projectile = true;
                        break;
                    }
                }
                enemy_slot = next_enemy;
            }
        }

        if (remove_projectile) {
            (void)game_state_delete_projectile(&app->state, projectile_slot);
        }
        projectile_slot = next_projectile;
    }
}

static void update_enemies(GameApp* app, float dt) {
    const Player* player = game_state_player_current_const(&app->state);
    if (!player) {
        return;
    }
    for (size_t enemy_slot = game_state_first_enemy(&app->state); enemy_slot != KEK_STATE_INVALID_ID; enemy_slot = game_state_next_enemy(&app->state, enemy_slot)) {
        const Enemy* enemy = game_state_enemy_at_const(&app->state, enemy_slot);
        if (!enemy || !enemy->active || game_state_is_declared_enemy_slot(&app->state, enemy_slot)) {
            continue;
        }
        EnemyUpdate update = {*player, dt, enemy_speed_for_kind(enemy->kind), 0.0f};
        (void)game_state_update_enemy_slot(&app->state, enemy_slot, enemy_update, &update,
                                           KEK_STATE_TYPE_ENEMY_FIELD_X |
                                               KEK_STATE_TYPE_ENEMY_FIELD_Y |
                                               KEK_STATE_TYPE_ENEMY_FIELD_VX |
                                               KEK_STATE_TYPE_ENEMY_FIELD_VY |
                                               KEK_STATE_TYPE_ENEMY_FIELD_FLASH);

        enemy = game_state_enemy_at_const(&app->state, enemy_slot);
        player = game_state_player_current_const(&app->state);
        if (!enemy || !enemy->active || !player || player->invulnerable_timer > 0.0f) {
            continue;
        }
        float distance = vec_len(player->x - enemy->x, player->y - enemy->y);
        if (distance <= PLAYER_RADIUS + enemy->radius) {
            PlayerDamageUpdate damage = {enemy->damage};
            float shake = 0.26f;
            (void)game_state_update_player(&app->state, player_damage_update, &damage,
                                           KEK_STATE_TYPE_PLAYER_FIELD_HEALTH |
                                               KEK_STATE_TYPE_PLAYER_FIELD_SHIELD |
                                               KEK_STATE_TYPE_PLAYER_FIELD_INVULNERABLE_TIMER);
            (void)game_state_update_session(&app->state, add_shake_update, &shake,
                                            KEK_STATE_TYPE_GAME_SESSION_FIELD_SHAKE);
            add_hud_message(app, HudMessageKind_Damage, player->x, player->y - 25.0f, damage.damage);
        }
    }
}

static void update_pickups(GameApp* app, float dt) {
    const Player* player = game_state_player_current_const(&app->state);
    if (!player) {
        return;
    }
    for (size_t pickup_slot = game_state_first_pickup(&app->state); pickup_slot != KEK_STATE_INVALID_ID;) {
        size_t next = game_state_next_pickup(&app->state, pickup_slot);
        const Pickup* pickup = game_state_pickup_at_const(&app->state, pickup_slot);
        if (!pickup) {
            pickup_slot = next;
            continue;
        }
        PickupUpdate update = {dt};
        (void)game_state_update_pickup_slot(&app->state, pickup_slot, pickup_update, &update,
                                            KEK_STATE_TYPE_PICKUP_FIELD_LIFE |
                                                KEK_STATE_TYPE_PICKUP_FIELD_PULSE);
        pickup = game_state_pickup_at_const(&app->state, pickup_slot);
        player = game_state_player_current_const(&app->state);
        if (!pickup || pickup->life <= 0.0f) {
            (void)game_state_delete_pickup(&app->state, pickup_slot);
        } else if (player && vec_len(player->x - pickup->x, player->y - pickup->y) <= PLAYER_RADIUS + PICKUP_RADIUS) {
            if (pickup->kind == PickupKind_Score) {
                ScoreUpdate score = {pickup->value, 0};
                (void)game_state_update_session(&app->state, score_update, &score,
                                                KEK_STATE_TYPE_GAME_SESSION_FIELD_SCORE);
                add_hud_message(app, HudMessageKind_Reward, pickup->x, pickup->y, pickup->value);
            } else {
                PlayerPickupUpdate reward = {pickup->kind, pickup->value};
                (void)game_state_update_player(&app->state, player_pickup_update, &reward,
                                               KEK_STATE_TYPE_PLAYER_FIELD_HEALTH |
                                                   KEK_STATE_TYPE_PLAYER_FIELD_SHIELD |
                                                   KEK_STATE_TYPE_PLAYER_FIELD_RAPID_TIMER);
                add_hud_message(app, HudMessageKind_Heal, pickup->x, pickup->y, pickup->value);
            }
            (void)game_state_delete_pickup(&app->state, pickup_slot);
        }
        pickup_slot = next;
    }
}

static void update_hud_messages(GameApp* app, float dt) {
    for (size_t slot = game_state_first_hud_message(&app->state); slot != KEK_STATE_INVALID_ID;) {
        size_t next = game_state_next_hud_message(&app->state, slot);
        const HudMessage* message = game_state_hud_message_at_const(&app->state, slot);
        if (!message) {
            slot = next;
            continue;
        }
        HudUpdate update = {dt};
        (void)game_state_update_hud_message_slot(&app->state, slot, hud_update, &update,
                                                 KEK_STATE_TYPE_HUD_MESSAGE_FIELD_Y |
                                                     KEK_STATE_TYPE_HUD_MESSAGE_FIELD_LIFE);
        message = game_state_hud_message_at_const(&app->state, slot);
        if (!message || message->life <= 0.0f) {
            (void)game_state_delete_hud_message(&app->state, slot);
        }
        slot = next;
    }
}

static void update_wave_runtime(GameApp* app, float dt) {
    WaveRuntimeUpdate update = {
        game_state_count_active_enemy(&app->state),
        dt,
    };
    (void)game_state_update_wave(&app->state, wave_runtime_update, &update,
                                 KEK_STATE_TYPE_WAVE_DIRECTOR_FIELD_ACTIVE_ENEMIES |
                                     KEK_STATE_TYPE_WAVE_DIRECTOR_FIELD_SPAWN_TIMER);
    const WaveDirector* wave = game_state_wave_current_const(&app->state);
    if (wave && wave->spawn_budget > 0 && wave->spawn_timer <= 0.0f) {
        spawn_enemy(app);
    }
}

static void update_playing(GameApp* app, InputIntent input, float dt) {
    const Player* player = game_state_player_current_const(&app->state);
    const GameSession* session = game_state_session_current_const(&app->state);
    if (!player || !session) {
        return;
    }

    if (input.pause) {
        GameMode pause = GameMode_Paused;
        (void)game_state_set_session_mode(&app->state, pause);
        return;
    }

    if (input.shoot && player->fire_cooldown <= 0.0f) {
        spawn_projectile(app, player, &input);
    }

    PlayerFrameUpdate player_update = {input, dt};
    SessionFrameUpdate session_update = {dt, IsKeyPressed(KEY_F3)};
    CameraFrameUpdate camera_update = {*player, dt};
    Game_stateUpdateItem updates[] = {
        game_state_player_update_item(&app->state, player_frame_update, &player_update,
                                      KEK_STATE_TYPE_PLAYER_FIELD_X | KEK_STATE_TYPE_PLAYER_FIELD_Y |
                                          KEK_STATE_TYPE_PLAYER_FIELD_VX | KEK_STATE_TYPE_PLAYER_FIELD_VY |
                                          KEK_STATE_TYPE_PLAYER_FIELD_FIRE_COOLDOWN |
                                          KEK_STATE_TYPE_PLAYER_FIELD_DASH_COOLDOWN |
                                          KEK_STATE_TYPE_PLAYER_FIELD_RAPID_TIMER |
                                          KEK_STATE_TYPE_PLAYER_FIELD_INVULNERABLE_TIMER),
        game_state_session_update_item(&app->state, session_frame_update, &session_update,
                                       KEK_STATE_TYPE_GAME_SESSION_FIELD_TIME_ALIVE |
                                           KEK_STATE_TYPE_GAME_SESSION_FIELD_COMBO |
                                           KEK_STATE_TYPE_GAME_SESSION_FIELD_COMBO_TIMER |
                                           KEK_STATE_TYPE_GAME_SESSION_FIELD_SHAKE |
                                           KEK_STATE_TYPE_GAME_SESSION_FIELD_DEBUG),
        game_state_camera_update_item(&app->state, camera_frame_update, &camera_update,
                                      KEK_STATE_TYPE_CAMERA_RIG_FIELD_X | KEK_STATE_TYPE_CAMERA_RIG_FIELD_Y |
                                          KEK_STATE_TYPE_CAMERA_RIG_FIELD_ZOOM),
    };
    (void)game_state_update_many(&app->state, updates, sizeof(updates) / sizeof(updates[0]));

    advance_frame_clock(app, dt);
    update_projectiles(app, dt);
    update_enemies(app, dt);
    update_pickups(app, dt);
    update_hud_messages(app, dt);
    update_wave_runtime(app, dt);
}

static void apply_upgrade(GameApp* app, int choice) {
    PlayerPickupUpdate pickup = {PickupKind_Shield, 25};
    if (choice == 1) {
        PlayerRewardUpdate level = {1000};
        (void)game_state_update_player(&app->state, player_reward_update, &level,
                                       KEK_STATE_TYPE_PLAYER_FIELD_XP |
                                           KEK_STATE_TYPE_PLAYER_FIELD_LEVEL |
                                           KEK_STATE_TYPE_PLAYER_FIELD_HEALTH |
                                           KEK_STATE_TYPE_PLAYER_FIELD_MAX_HEALTH);
    } else if (choice == 2) {
        pickup.kind = PickupKind_RapidFire;
        pickup.value = 1;
        (void)game_state_update_player(&app->state, player_pickup_update, &pickup,
                                       KEK_STATE_TYPE_PLAYER_FIELD_RAPID_TIMER);
    } else {
        (void)game_state_update_player(&app->state, player_pickup_update, &pickup,
                                       KEK_STATE_TYPE_PLAYER_FIELD_SHIELD);
    }
    GameMode resume = GameMode_Playing;
    (void)game_state_set_session_mode(&app->state, resume);
}

static void update_game(GameApp* app, float dt) {
    InputIntent input = collect_input(app);
    (void)game_state_update_input(&app->state, input_update, &input,
                                  KEK_EVENT_CHANGED_FIELDS_UNKNOWN);

    const GameSession* session = game_state_session_current_const(&app->state);
    if (!session) {
        return;
    }

    if (WindowShouldClose()) {
        app->should_quit = true;
        return;
    }

    if (session->mode == GameMode_Menu) {
        if (input.confirm) {
            start_game(app);
        }
    } else if (session->mode == GameMode_Playing) {
        update_playing(app, input, dt);
    } else if (session->mode == GameMode_Paused) {
        if (input.pause || input.confirm) {
            GameMode resume = GameMode_Playing;
            (void)game_state_set_session_mode(&app->state, resume);
        }
    } else if (session->mode == GameMode_Upgrade) {
        if (IsKeyPressed(KEY_ONE)) {
            apply_upgrade(app, 1);
        } else if (IsKeyPressed(KEY_TWO)) {
            apply_upgrade(app, 2);
        } else if (IsKeyPressed(KEY_THREE)) {
            apply_upgrade(app, 3);
        }
    } else if ((session->mode == GameMode_GameOver || session->mode == GameMode_Victory) && input.confirm) {
        start_game(app);
    }

    (void)game_state_dispatch(&app->state);
}
