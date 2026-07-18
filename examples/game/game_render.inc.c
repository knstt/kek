static Color enemy_color(const Enemy* enemy) {
    if (enemy->flash > 0.0f) {
        return WHITE;
    }
    if (enemy->kind == EnemyKind_Runner) {
        return (Color){255, 105, 97, 255};
    }
    if (enemy->kind == EnemyKind_Tank) {
        return (Color){166, 108, 255, 255};
    }
    if (enemy->kind == EnemyKind_Boss) {
        return (Color){255, 202, 58, 255};
    }
    return (Color){255, 77, 109, 255};
}

static Color pickup_color(PickupKind kind) {
    if (kind == PickupKind_Health) {
        return (Color){102, 255, 153, 255};
    }
    if (kind == PickupKind_RapidFire) {
        return (Color){97, 218, 251, 255};
    }
    if (kind == PickupKind_Shield) {
        return (Color){116, 140, 255, 255};
    }
    return GOLD;
}

static Color hud_color(HudMessageKind kind) {
    if (kind == HudMessageKind_Damage) {
        return RED;
    }
    if (kind == HudMessageKind_Heal) {
        return GREEN;
    }
    if (kind == HudMessageKind_Reward) {
        return GOLD;
    }
    if (kind == HudMessageKind_Warning) {
        return ORANGE;
    }
    return RAYWHITE;
}

static void draw_world(GameApp* app) {
    const Player* player = game_state_player_current_const(&app->state);
    const InputIntent* input = game_state_input_current_const(&app->state);
    if (!player) {
        return;
    }

    DrawRectangleLinesEx((Rectangle){-ARENA_HALF_WIDTH, -ARENA_HALF_HEIGHT,
                                     ARENA_HALF_WIDTH * 2.0f, ARENA_HALF_HEIGHT * 2.0f},
                         4.0f, (Color){52, 63, 96, 255});
    for (int x = -900; x <= 900; x += 120) {
        DrawLine(x, (int)-ARENA_HALF_HEIGHT, x, (int)ARENA_HALF_HEIGHT, (Color){26, 31, 50, 255});
    }
    for (int y = -480; y <= 480; y += 120) {
        DrawLine((int)-ARENA_HALF_WIDTH, y, (int)ARENA_HALF_WIDTH, y, (Color){26, 31, 50, 255});
    }

    for (size_t slot = game_state_first_pickup(&app->state); slot != KEK_STATE_INVALID_ID; slot = game_state_next_pickup(&app->state, slot)) {
        const Pickup* pickup = game_state_pickup_at_const(&app->state, slot);
        if (!pickup) {
            continue;
        }
        float radius = PICKUP_RADIUS + sinf(pickup->pulse) * 2.0f;
        DrawCircleV((Vector2){pickup->x, pickup->y}, radius, pickup_color(pickup->kind));
        DrawCircleLines((int)pickup->x, (int)pickup->y, radius + 4.0f, Fade(WHITE, 0.35f));
    }

    for (size_t slot = game_state_first_projectile(&app->state); slot != KEK_STATE_INVALID_ID; slot = game_state_next_projectile(&app->state, slot)) {
        const Projectile* projectile = game_state_projectile_at_const(&app->state, slot);
        if (projectile) {
            DrawCircleV((Vector2){projectile->x, projectile->y}, PROJECTILE_RADIUS, SKYBLUE);
            DrawCircleV((Vector2){projectile->x, projectile->y}, PROJECTILE_RADIUS * 0.45f, WHITE);
        }
    }

    for (size_t slot = game_state_first_enemy(&app->state); slot != KEK_STATE_INVALID_ID; slot = game_state_next_enemy(&app->state, slot)) {
        const Enemy* enemy = game_state_enemy_at_const(&app->state, slot);
        if (!enemy || !enemy->active) {
            continue;
        }
        DrawCircleV((Vector2){enemy->x, enemy->y}, enemy->radius, enemy_color(enemy));
        DrawCircleLines((int)enemy->x, (int)enemy->y, enemy->radius + 3.0f, Fade(BLACK, 0.4f));
        float health_ratio = clampf((float)enemy->health / (enemy->kind == EnemyKind_Boss ? 500.0f : 90.0f), 0.0f, 1.0f);
        DrawRectangle((int)(enemy->x - enemy->radius), (int)(enemy->y - enemy->radius - 12.0f),
                      (int)(enemy->radius * 2.0f * health_ratio), 4, RED);
    }

    float gun_x = input ? input->aim_x : 1.0f;
    float gun_y = input ? input->aim_y : 0.0f;
    normalize_or(&gun_x, &gun_y, 1.0f, 0.0f);
    Vector2 barrel_start = {player->x + gun_x * (PLAYER_RADIUS - 3.0f),
                            player->y + gun_y * (PLAYER_RADIUS - 3.0f)};
    Vector2 barrel_end = {player->x + gun_x * (PLAYER_RADIUS + 22.0f),
                          player->y + gun_y * (PLAYER_RADIUS + 22.0f)};
    DrawLineEx(barrel_start, barrel_end, 8.0f, (Color){190, 205, 225, 255});
    DrawCircleV(barrel_end, 4.5f, WHITE);

    Color player_color = player->invulnerable_timer > 0.0f ? Fade(SKYBLUE, 0.55f) : (Color){86, 232, 255, 255};
    DrawCircleV((Vector2){player->x, player->y}, PLAYER_RADIUS, player_color);
    DrawCircleLines((int)player->x, (int)player->y, PLAYER_RADIUS + 5.0f, Fade(WHITE, 0.55f));
}

static void draw_overlay(GameApp* app, Camera2D view) {
    const GameSession* session = game_state_session_current_const(&app->state);
    const Player* player = game_state_player_current_const(&app->state);
    const WaveDirector* wave = game_state_wave_current_const(&app->state);
    if (!session || !player || !wave) {
        return;
    }

    DrawRectangle(18, 18, 370, 112, Fade((Color){9, 12, 24, 255}, 0.82f));
    DrawText(TextFormat("HP %d/%d  SH %d", player->health, player->max_health, player->shield), 34, 30, 22, RAYWHITE);
    DrawRectangle(34, 62, 190, 12, Fade(RED, 0.25f));
    DrawRectangle(34, 62, (int)(190.0f * (float)player->health / (float)player->max_health), 12, RED);
    DrawText(TextFormat("LV %d XP %d  SCORE %d", player->level, player->xp, session->score), 34, 82, 20, GOLD);
    DrawText(TextFormat("WAVE %d  LEFT %d  COMBO x%d", wave->wave, wave->spawn_budget + wave->active_enemies, session->combo),
             34, 105, 18, SKYBLUE);

    for (size_t slot = game_state_first_hud_message(&app->state); slot != KEK_STATE_INVALID_ID; slot = game_state_next_hud_message(&app->state, slot)) {
        const HudMessage* message = game_state_hud_message_at_const(&app->state, slot);
        if (!message) {
            continue;
        }
        Vector2 screen = GetWorldToScreen2D((Vector2){message->x, message->y}, view);
        const char* text = "+";
        if (message->kind == HudMessageKind_Damage) {
            text = "-";
        }
        DrawText(TextFormat("%s%d", text, message->value), (int)screen.x,
                 (int)screen.y, 18, Fade(hud_color(message->kind), message->life));
    }

    if (session->debug) {
        DrawRectangle(SCREEN_WIDTH - 318, 18, 300, 120, Fade(BLACK, 0.75f));
        DrawText(TextFormat("FPS %d", GetFPS()), SCREEN_WIDTH - 300, 30, 18, GREEN);
        DrawText(TextFormat("Enemies %d Projectiles %d", game_state_count_active_enemy(&app->state),
                            game_state_count_projectile(&app->state)),
                 SCREEN_WIDTH - 300, 54, 18, RAYWHITE);
        DrawText(TextFormat("Pickups %d HUD %d", game_state_count_pickup(&app->state),
                            game_state_count_hud_message(&app->state)),
                 SCREEN_WIDTH - 300, 78, 18, RAYWHITE);
        DrawText(TextFormat("Time %.1f", session->time_alive), SCREEN_WIDTH - 300, 102, 18, RAYWHITE);
    }
}

static void draw_center_panel(const char* title, const char* body, const char* hint, Color accent) {
    Rectangle panel = {SCREEN_WIDTH * 0.5f - 310.0f, SCREEN_HEIGHT * 0.5f - 150.0f, 620.0f, 300.0f};
    DrawRectangleRounded(panel, 0.08f, 12, Fade((Color){8, 12, 25, 255}, 0.94f));
    DrawRectangleRoundedLines(panel, 0.08f, 12, accent);
    DrawText(title, (int)panel.x + 38, (int)panel.y + 34, 42, accent);
    DrawText(body, (int)panel.x + 42, (int)panel.y + 104, 21, RAYWHITE);
    DrawText(hint, (int)panel.x + 42, (int)panel.y + 232, 22, GOLD);
}

static Camera2D camera_for(GameApp* app) {
    const CameraRig* camera = game_state_camera_current_const(&app->state);
    const GameSession* session = game_state_session_current_const(&app->state);
    Camera2D view = {0};
    view.target = camera ? (Vector2){camera->x, camera->y} : (Vector2){0.0f, 0.0f};
    view.offset = (Vector2){SCREEN_WIDTH * 0.5f, SCREEN_HEIGHT * 0.5f};
    view.rotation = 0.0f;
    view.zoom = camera ? camera->zoom : 1.0f;
    if (session && session->shake > 0.0f) {
        float shake = session->shake * session->shake * 12.0f;
        view.offset.x += (float)GetRandomValue(-100, 100) * 0.01f * shake;
        view.offset.y += (float)GetRandomValue(-100, 100) * 0.01f * shake;
    }
    return view;
}

static void draw_game(GameApp* app) {
    const GameSession* session = game_state_session_current_const(&app->state);
    BeginDrawing();
    ClearBackground((Color){11, 15, 30, 255});

    Camera2D view = camera_for(app);
    BeginMode2D(view);
    draw_world(app);
    EndMode2D();
    draw_overlay(app, view);

    if (session) {
        if (session->mode == GameMode_Menu) {
            draw_center_panel("KEK RAY SURVIVOR",
                              "Generated schema state drives the session, player, waves,\n"
                              "dynamic enemies, bullets, pickups, HUD messages and hooks.",
                              "ENTER or SPACE to start", SKYBLUE);
        } else if (session->mode == GameMode_Paused) {
            draw_center_panel("PAUSED", "The state store is frozen while rendering continues.",
                              "P, ESC, ENTER or SPACE to resume", ORANGE);
        } else if (session->mode == GameMode_Upgrade) {
            draw_center_panel("UPGRADE READY",
                              "1: Field Training - instant level\n2: Overclock - rapid fire\n3: Capacitor - shield boost",
                              "Choose 1, 2, or 3", GREEN);
        } else if (session->mode == GameMode_GameOver) {
            draw_center_panel("GAME OVER", TextFormat("Final score: %d", session->score),
                              "ENTER or SPACE to restart", RED);
        } else if (session->mode == GameMode_Victory) {
            draw_center_panel("VICTORY", TextFormat("Arena cleared. Final score: %d", session->score),
                              "ENTER or SPACE to restart", GOLD);
        }
    }

    DrawText("WASD move  Arrows steer gun  Space shoot  Left Shift dash  P pause  F3 debug",
             24, SCREEN_HEIGHT - 32, 18, Fade(RAYWHITE, 0.72f));
    EndDrawing();
}
