#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "raylib.h"

#include "examples/game/generated/game_state.h"

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
#define ARENA_HALF_WIDTH 920.0f
#define ARENA_HALF_HEIGHT 520.0f
#define PLAYER_RADIUS 18.0f
#define PROJECTILE_RADIUS 6.0f
#define PICKUP_RADIUS 13.0f
#define HUD_VALUE_DAMAGE 0
#define HUD_VALUE_HEAL 1
#define HUD_VALUE_REWARD 2
#define HUD_VALUE_WARNING 3

typedef struct GameApp {
    KekRuntime runtime;
    Game_stateRuntimeBinding binding;
    float timer_accumulator;
    bool should_quit;
} GameApp;

typedef struct ModeUpdate {
    GameMode mode;
    int next_upgrade_score;
} ModeUpdate;

typedef struct SessionFrameUpdate {
    float dt;
    bool toggle_debug;
} SessionFrameUpdate;

typedef struct ScoreUpdate {
    int score_delta;
    int combo_delta;
} ScoreUpdate;

typedef struct PlayerFrameUpdate {
    InputIntent input;
    float dt;
} PlayerFrameUpdate;

typedef struct PlayerDamageUpdate {
    int damage;
} PlayerDamageUpdate;

typedef struct PlayerRewardUpdate {
    int xp;
} PlayerRewardUpdate;

typedef struct PlayerPickupUpdate {
    PickupKind kind;
    int value;
} PlayerPickupUpdate;

typedef struct CameraFrameUpdate {
    Player player;
    float dt;
} CameraFrameUpdate;

typedef struct TimerTickUpdate {
    uint64_t tick;
} TimerTickUpdate;

typedef struct WaveRuntimeUpdate {
    int active_enemies;
    float dt;
} WaveRuntimeUpdate;

typedef struct WaveFullUpdate {
    WaveDirector wave;
} WaveFullUpdate;

typedef struct WaveSpawnUpdate {
    float next_spawn_timer;
    bool boss_spawned;
} WaveSpawnUpdate;

typedef struct ProjectileUpdate {
    float dt;
} ProjectileUpdate;

typedef struct EnemyUpdate {
    Player player;
    float dt;
} EnemyUpdate;

typedef struct EnemyDamageUpdate {
    int damage;
} EnemyDamageUpdate;

typedef struct PickupUpdate {
    float dt;
} PickupUpdate;

typedef struct HudUpdate {
    float dt;
} HudUpdate;

static Camera2D camera_for(GameApp* app);

static float clampf(float value, float minimum, float maximum) {
    if (value < minimum) {
        return minimum;
    }
    if (value > maximum) {
        return maximum;
    }
    return value;
}

static float vec_len(float x, float y) {
    return sqrtf(x * x + y * y);
}

static void normalize_or(float* x, float* y, float fallback_x, float fallback_y) {
    float length = vec_len(*x, *y);
    if (length > 0.0001f) {
        *x /= length;
        *y /= length;
        return;
    }
    *x = fallback_x;
    *y = fallback_y;
}

static KekStateStore* game_store(GameApp* app) {
    return &app->binding.state_store;
}

static const GameSession* session_const(GameApp* app) {
    return game_state_session_const(game_store(app), &app->binding.slots);
}

static const Player* player_const(GameApp* app) {
    return game_state_player_const(game_store(app), &app->binding.slots);
}

static const WaveDirector* wave_const(GameApp* app) {
    return game_state_wave_const(game_store(app), &app->binding.slots);
}

static void set_mode_update(void* draft, void* context) {
    GameSession* session = (GameSession*)draft;
    const ModeUpdate* update = (const ModeUpdate*)context;
    session->mode = update->mode;
    session->next_upgrade_score = update->next_upgrade_score;
}

static void set_mode_only_update(void* draft, void* context) {
    GameSession* session = (GameSession*)draft;
    const GameMode* mode = (const GameMode*)context;
    session->mode = *mode;
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

static void timer_start_update(void* draft, void* context) {
    (void)context;
    Timer* timer = (Timer*)draft;
    timer->tick = 0;
    timer->interval_ms = 250;
    timer->enabled = true;
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

static void player_fire_cooldown_update(void* draft, void* context) {
    Player* player = (Player*)draft;
    const float* cooldown = (const float*)context;
    player->fire_cooldown = *cooldown;
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

static void timer_tick_update(void* draft, void* context) {
    Timer* timer = (Timer*)draft;
    const TimerTickUpdate* update = (const TimerTickUpdate*)context;
    timer->tick = update->tick;
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
    float dir_x = update->player.x - enemy->x;
    float dir_y = update->player.y - enemy->y;
    normalize_or(&dir_x, &dir_y, 0.0f, 0.0f);

    float speed = 108.0f;
    if (enemy->kind == EnemyKind_Runner) {
        speed = 172.0f;
    } else if (enemy->kind == EnemyKind_Tank) {
        speed = 76.0f;
    } else if (enemy->kind == EnemyKind_Boss) {
        speed = 58.0f;
    }

    enemy->vx = dir_x * speed;
    enemy->vy = dir_y * speed;
    enemy->x = clampf(enemy->x + enemy->vx * update->dt, -ARENA_HALF_WIDTH - 160.0f, ARENA_HALF_WIDTH + 160.0f);
    enemy->y = clampf(enemy->y + enemy->vy * update->dt, -ARENA_HALF_HEIGHT - 160.0f, ARENA_HALF_HEIGHT + 160.0f);
    enemy->flash = clampf(enemy->flash - update->dt * 4.5f, 0.0f, 1.0f);
}

static void enemy_damage_update(void* draft, void* context) {
    Enemy* enemy = (Enemy*)draft;
    const EnemyDamageUpdate* update = (const EnemyDamageUpdate*)context;
    enemy->health -= update->damage;
    if (enemy->health < 0) {
        enemy->health = 0;
    }
    enemy->flash = 1.0f;
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

static int count_slots(KekStateStore* store, size_t (*first)(const KekStateStore*),
                       size_t (*next)(const KekStateStore*, size_t)) {
    int count = 0;
    for (size_t slot = first(store); slot != KEK_STATE_INVALID_ID; slot = next(store, slot)) {
        count++;
    }
    return count;
}

static void clear_dynamic_entities(GameApp* app) {
    KekStateStore* store = game_store(app);
    for (size_t slot = game_state_enemy_first(store); slot != KEK_STATE_INVALID_ID;) {
        size_t next = game_state_enemy_next(store, slot);
        (void)game_state_enemy_delete(store, slot);
        slot = next;
    }
    for (size_t slot = game_state_projectile_first(store); slot != KEK_STATE_INVALID_ID;) {
        size_t next = game_state_projectile_next(store, slot);
        (void)game_state_projectile_delete(store, slot);
        slot = next;
    }
    for (size_t slot = game_state_pickup_first(store); slot != KEK_STATE_INVALID_ID;) {
        size_t next = game_state_pickup_next(store, slot);
        (void)game_state_pickup_delete(store, slot);
        slot = next;
    }
    for (size_t slot = game_state_hud_message_first(store); slot != KEK_STATE_INVALID_ID;) {
        size_t next = game_state_hud_message_next(store, slot);
        (void)game_state_hud_message_delete(store, slot);
        slot = next;
    }
}

static void add_hud_message(GameApp* app, HudMessageKind kind, float x, float y, int value) {
    HudMessage message = HudMessage_default();
    message.kind = kind;
    message.x = x;
    message.y = y;
    message.value = value;
    message.life = 1.3f;
    (void)game_state_hud_message_create_with(game_store(app), &message);
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
    (void)game_state_pickup_create_with(game_store(app), &pickup);
}

static void start_game(GameApp* app) {
    clear_dynamic_entities(app);
    app->timer_accumulator = 0.0f;
    KekStateStoreUpdateItem updates[] = {
        {app->binding.slots.session, session_start_update, NULL, KEK_EVENT_CHANGED_FIELDS_UNKNOWN},
        {app->binding.slots.player, player_start_update, NULL, KEK_EVENT_CHANGED_FIELDS_UNKNOWN},
        {app->binding.slots.wave, wave_start_update, NULL, KEK_EVENT_CHANGED_FIELDS_UNKNOWN},
        {app->binding.slots.camera, camera_start_update, NULL, KEK_EVENT_CHANGED_FIELDS_UNKNOWN},
        {app->binding.slots.timer, timer_start_update, NULL, KEK_EVENT_CHANGED_FIELDS_UNKNOWN},
    };
    (void)kek_state_store_update_many(game_store(app), updates, sizeof(updates) / sizeof(updates[0]));
    add_hud_message(app, HudMessageKind_Info, 0.0f, -60.0f, HUD_VALUE_WARNING);
    (void)kek_event_dispatch_pending(kek_runtime_events(&app->runtime));
}

static InputIntent collect_input(GameApp* app) {
    const InputIntent* previous_input = game_state_input_const(game_store(app), &app->binding.slots);
    const Player* player = player_const(app);
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
    (void)game_state_projectile_create_with(game_store(app), &projectile);

    float cooldown = player->rapid_timer > 0.0f ? 0.095f : 0.18f;
    (void)kek_state_store_update_fields(game_store(app), app->binding.slots.player,
                                        player_fire_cooldown_update, &cooldown,
                                        KEK_STATE_TYPE_PLAYER_FIELD_FIRE_COOLDOWN);
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
    const WaveDirector* wave = wave_const(app);
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
    (void)game_state_enemy_create_with(game_store(app), &enemy);
    WaveSpawnUpdate update = {
        clampf(0.72f - (float)wave->wave * 0.055f, 0.22f, 0.72f),
        boss ? true : wave->boss_spawned,
    };
    (void)kek_state_store_update_fields(game_store(app), app->binding.slots.wave,
                                        wave_spawn_update, &update,
                                        KEK_STATE_TYPE_WAVE_DIRECTOR_FIELD_SPAWN_BUDGET |
                                            KEK_STATE_TYPE_WAVE_DIRECTOR_FIELD_SPAWN_TIMER |
                                            KEK_STATE_TYPE_WAVE_DIRECTOR_FIELD_BOSS_SPAWNED);
}

static void advance_timer(GameApp* app, float dt) {
    Timer* timer = game_state_timer(game_store(app), &app->binding.slots);
    if (!timer || !timer->enabled || timer->interval_ms == 0) {
        return;
    }
    app->timer_accumulator += dt;
    float interval = (float)timer->interval_ms / 1000.0f;
    while (app->timer_accumulator >= interval) {
        app->timer_accumulator -= interval;
        TimerTickUpdate update = {timer->tick + 1};
        (void)kek_state_store_update_fields(game_store(app), app->binding.slots.timer,
                                            timer_tick_update, &update,
                                            KEK_STATE_TYPE_TIMER_FIELD_TICK);
        (void)kek_event_dispatch_pending(kek_runtime_events(&app->runtime));
        timer = game_state_timer(game_store(app), &app->binding.slots);
        if (!timer) {
            return;
        }
    }
}

static void update_projectiles(GameApp* app, float dt) {
    KekStateStore* store = game_store(app);
    for (size_t projectile_slot = game_state_projectile_first(store); projectile_slot != KEK_STATE_INVALID_ID;) {
        size_t next_projectile = game_state_projectile_next(store, projectile_slot);
        const Projectile* projectile = game_state_projectile_slot_const(store, projectile_slot);
        if (!projectile) {
            projectile_slot = next_projectile;
            continue;
        }

        bool remove_projectile = projectile->life <= 0.0f || projectile->pierce <= 0 ||
                                 fabsf(projectile->x) > ARENA_HALF_WIDTH + 220.0f ||
                                 fabsf(projectile->y) > ARENA_HALF_HEIGHT + 220.0f;
        if (!remove_projectile) {
            ProjectileUpdate update = {dt};
            (void)kek_state_store_update_fields(store, projectile_slot, projectile_update, &update,
                                                KEK_STATE_TYPE_PROJECTILE_FIELD_X |
                                                    KEK_STATE_TYPE_PROJECTILE_FIELD_Y |
                                                    KEK_STATE_TYPE_PROJECTILE_FIELD_LIFE);
            projectile = game_state_projectile_slot_const(store, projectile_slot);
        }

        if (!remove_projectile && projectile) {
            for (size_t enemy_slot = game_state_enemy_first(store); enemy_slot != KEK_STATE_INVALID_ID;) {
                size_t next_enemy = game_state_enemy_next(store, enemy_slot);
                const Enemy* enemy = game_state_enemy_slot_const(store, enemy_slot);
                projectile = game_state_projectile_slot_const(store, projectile_slot);
                if (!enemy || !projectile) {
                    enemy_slot = next_enemy;
                    continue;
                }
                float distance = vec_len(projectile->x - enemy->x, projectile->y - enemy->y);
                if (distance <= enemy->radius + PROJECTILE_RADIUS) {
                    EnemyDamageUpdate hit = {projectile->damage};
                    (void)kek_state_store_update_fields(store, enemy_slot, enemy_damage_update, &hit,
                                                        KEK_STATE_TYPE_ENEMY_FIELD_HEALTH |
                                                            KEK_STATE_TYPE_ENEMY_FIELD_FLASH);
                    (void)kek_state_store_update_fields(store, projectile_slot, projectile_pierce_update,
                                                        NULL, KEK_STATE_TYPE_PROJECTILE_FIELD_PIERCE);
                    enemy = game_state_enemy_slot_const(store, enemy_slot);
                    projectile = game_state_projectile_slot_const(store, projectile_slot);
                    if (enemy && enemy->health <= 0) {
                        int reward = enemy->kind == EnemyKind_Boss ? 1000 : 35 + (int)enemy->radius;
                        ScoreUpdate score = {reward, 1};
                        PlayerRewardUpdate xp = {enemy->kind == EnemyKind_Boss ? 140 : 16};
                        (void)kek_state_store_update_fields(store, app->binding.slots.session,
                                                            score_update, &score,
                                                            KEK_STATE_TYPE_GAME_SESSION_FIELD_SCORE |
                                                                KEK_STATE_TYPE_GAME_SESSION_FIELD_COMBO |
                                                                KEK_STATE_TYPE_GAME_SESSION_FIELD_COMBO_TIMER);
                        (void)kek_state_store_update_fields(store, app->binding.slots.player,
                                                            player_reward_update, &xp,
                                                            KEK_STATE_TYPE_PLAYER_FIELD_XP |
                                                                KEK_STATE_TYPE_PLAYER_FIELD_LEVEL |
                                                                KEK_STATE_TYPE_PLAYER_FIELD_HEALTH |
                                                                KEK_STATE_TYPE_PLAYER_FIELD_MAX_HEALTH);
                        add_pickup(app, enemy->x, enemy->y, reward / 3);
                        add_hud_message(app, HudMessageKind_Reward, enemy->x, enemy->y, reward);
                        (void)game_state_enemy_delete(store, enemy_slot);
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
            (void)game_state_projectile_delete(store, projectile_slot);
        }
        projectile_slot = next_projectile;
    }
}

static void update_enemies(GameApp* app, float dt) {
    KekStateStore* store = game_store(app);
    const Player* player = player_const(app);
    if (!player) {
        return;
    }
    for (size_t enemy_slot = game_state_enemy_first(store); enemy_slot != KEK_STATE_INVALID_ID; enemy_slot = game_state_enemy_next(store, enemy_slot)) {
        const Enemy* enemy = game_state_enemy_slot_const(store, enemy_slot);
        if (!enemy) {
            continue;
        }
        EnemyUpdate update = {*player, dt};
        (void)kek_state_store_update_fields(store, enemy_slot, enemy_update, &update,
                                            KEK_STATE_TYPE_ENEMY_FIELD_X |
                                                KEK_STATE_TYPE_ENEMY_FIELD_Y |
                                                KEK_STATE_TYPE_ENEMY_FIELD_VX |
                                                KEK_STATE_TYPE_ENEMY_FIELD_VY |
                                                KEK_STATE_TYPE_ENEMY_FIELD_FLASH);

        enemy = game_state_enemy_slot_const(store, enemy_slot);
        player = player_const(app);
        if (!enemy || !player || player->invulnerable_timer > 0.0f) {
            continue;
        }
        float distance = vec_len(player->x - enemy->x, player->y - enemy->y);
        if (distance <= PLAYER_RADIUS + enemy->radius) {
            PlayerDamageUpdate damage = {enemy->damage};
            float shake = 0.26f;
            (void)kek_state_store_update_fields(store, app->binding.slots.player,
                                                player_damage_update, &damage,
                                                KEK_STATE_TYPE_PLAYER_FIELD_HEALTH |
                                                    KEK_STATE_TYPE_PLAYER_FIELD_SHIELD |
                                                    KEK_STATE_TYPE_PLAYER_FIELD_INVULNERABLE_TIMER);
            (void)kek_state_store_update_fields(store, app->binding.slots.session,
                                                add_shake_update, &shake,
                                                KEK_STATE_TYPE_GAME_SESSION_FIELD_SHAKE);
            add_hud_message(app, HudMessageKind_Damage, player->x, player->y - 25.0f, damage.damage);
        }
    }
}

static void update_pickups(GameApp* app, float dt) {
    KekStateStore* store = game_store(app);
    const Player* player = player_const(app);
    if (!player) {
        return;
    }
    for (size_t pickup_slot = game_state_pickup_first(store); pickup_slot != KEK_STATE_INVALID_ID;) {
        size_t next = game_state_pickup_next(store, pickup_slot);
        const Pickup* pickup = game_state_pickup_slot_const(store, pickup_slot);
        if (!pickup) {
            pickup_slot = next;
            continue;
        }
        PickupUpdate update = {dt};
        (void)kek_state_store_update_fields(store, pickup_slot, pickup_update, &update,
                                            KEK_STATE_TYPE_PICKUP_FIELD_LIFE |
                                                KEK_STATE_TYPE_PICKUP_FIELD_PULSE);
        pickup = game_state_pickup_slot_const(store, pickup_slot);
        player = player_const(app);
        if (!pickup || pickup->life <= 0.0f) {
            (void)game_state_pickup_delete(store, pickup_slot);
        } else if (player && vec_len(player->x - pickup->x, player->y - pickup->y) <= PLAYER_RADIUS + PICKUP_RADIUS) {
            if (pickup->kind == PickupKind_Score) {
                ScoreUpdate score = {pickup->value, 0};
                (void)kek_state_store_update_fields(store, app->binding.slots.session,
                                                    score_update, &score,
                                                    KEK_STATE_TYPE_GAME_SESSION_FIELD_SCORE);
                add_hud_message(app, HudMessageKind_Reward, pickup->x, pickup->y, pickup->value);
            } else {
                PlayerPickupUpdate reward = {pickup->kind, pickup->value};
                (void)kek_state_store_update_fields(store, app->binding.slots.player,
                                                    player_pickup_update, &reward,
                                                    KEK_STATE_TYPE_PLAYER_FIELD_HEALTH |
                                                        KEK_STATE_TYPE_PLAYER_FIELD_SHIELD |
                                                        KEK_STATE_TYPE_PLAYER_FIELD_RAPID_TIMER);
                add_hud_message(app, HudMessageKind_Heal, pickup->x, pickup->y, pickup->value);
            }
            (void)game_state_pickup_delete(store, pickup_slot);
        }
        pickup_slot = next;
    }
}

static void update_hud_messages(GameApp* app, float dt) {
    KekStateStore* store = game_store(app);
    for (size_t slot = game_state_hud_message_first(store); slot != KEK_STATE_INVALID_ID;) {
        size_t next = game_state_hud_message_next(store, slot);
        const HudMessage* message = game_state_hud_message_slot_const(store, slot);
        if (!message) {
            slot = next;
            continue;
        }
        HudUpdate update = {dt};
        (void)kek_state_store_update_fields(store, slot, hud_update, &update,
                                            KEK_STATE_TYPE_HUD_MESSAGE_FIELD_Y |
                                                KEK_STATE_TYPE_HUD_MESSAGE_FIELD_LIFE);
        message = game_state_hud_message_slot_const(store, slot);
        if (!message || message->life <= 0.0f) {
            (void)game_state_hud_message_delete(store, slot);
        }
        slot = next;
    }
}

static void update_wave_runtime(GameApp* app, float dt) {
    WaveRuntimeUpdate update = {
        count_slots(game_store(app), game_state_enemy_first, game_state_enemy_next),
        dt,
    };
    (void)kek_state_store_update_fields(game_store(app), app->binding.slots.wave,
                                        wave_runtime_update, &update,
                                        KEK_STATE_TYPE_WAVE_DIRECTOR_FIELD_ACTIVE_ENEMIES |
                                            KEK_STATE_TYPE_WAVE_DIRECTOR_FIELD_SPAWN_TIMER);
    const WaveDirector* wave = wave_const(app);
    if (wave && wave->spawn_budget > 0 && wave->spawn_timer <= 0.0f) {
        spawn_enemy(app);
    }
}

static void update_playing(GameApp* app, InputIntent input, float dt) {
    KekStateStore* store = game_store(app);
    const Player* player = player_const(app);
    const GameSession* session = session_const(app);
    if (!player || !session) {
        return;
    }

    if (input.pause) {
        GameMode pause = GameMode_Paused;
        (void)kek_state_store_update_fields(store, app->binding.slots.session,
                                            set_mode_only_update, &pause,
                                            KEK_STATE_TYPE_GAME_SESSION_FIELD_MODE);
        return;
    }

    if (input.shoot && player->fire_cooldown <= 0.0f) {
        spawn_projectile(app, player, &input);
    }

    PlayerFrameUpdate player_update = {input, dt};
    SessionFrameUpdate session_update = {dt, IsKeyPressed(KEY_F3)};
    CameraFrameUpdate camera_update = {*player, dt};
    KekStateStoreUpdateItem updates[] = {
        {app->binding.slots.player, player_frame_update, &player_update,
         KEK_STATE_TYPE_PLAYER_FIELD_X | KEK_STATE_TYPE_PLAYER_FIELD_Y |
             KEK_STATE_TYPE_PLAYER_FIELD_VX | KEK_STATE_TYPE_PLAYER_FIELD_VY |
             KEK_STATE_TYPE_PLAYER_FIELD_FIRE_COOLDOWN |
             KEK_STATE_TYPE_PLAYER_FIELD_DASH_COOLDOWN |
             KEK_STATE_TYPE_PLAYER_FIELD_RAPID_TIMER |
             KEK_STATE_TYPE_PLAYER_FIELD_INVULNERABLE_TIMER},
        {app->binding.slots.session, session_frame_update, &session_update,
         KEK_STATE_TYPE_GAME_SESSION_FIELD_TIME_ALIVE |
             KEK_STATE_TYPE_GAME_SESSION_FIELD_COMBO |
             KEK_STATE_TYPE_GAME_SESSION_FIELD_COMBO_TIMER |
             KEK_STATE_TYPE_GAME_SESSION_FIELD_SHAKE |
             KEK_STATE_TYPE_GAME_SESSION_FIELD_DEBUG},
        {app->binding.slots.camera, camera_frame_update, &camera_update,
          KEK_STATE_TYPE_CAMERA_RIG_FIELD_X | KEK_STATE_TYPE_CAMERA_RIG_FIELD_Y |
              KEK_STATE_TYPE_CAMERA_RIG_FIELD_ZOOM},
    };
    (void)kek_state_store_update_many(store, updates, sizeof(updates) / sizeof(updates[0]));

    advance_timer(app, dt);
    update_projectiles(app, dt);
    update_enemies(app, dt);
    update_pickups(app, dt);
    update_hud_messages(app, dt);
    update_wave_runtime(app, dt);
}

static void apply_upgrade(GameApp* app, int choice) {
    KekStateStore* store = game_store(app);
    PlayerPickupUpdate pickup = {PickupKind_Shield, 25};
    if (choice == 1) {
        PlayerRewardUpdate level = {1000};
        (void)kek_state_store_update_fields(store, app->binding.slots.player,
                                            player_reward_update, &level,
                                            KEK_STATE_TYPE_PLAYER_FIELD_XP |
                                                KEK_STATE_TYPE_PLAYER_FIELD_LEVEL |
                                                KEK_STATE_TYPE_PLAYER_FIELD_HEALTH |
                                                KEK_STATE_TYPE_PLAYER_FIELD_MAX_HEALTH);
    } else if (choice == 2) {
        pickup.kind = PickupKind_RapidFire;
        pickup.value = 1;
        (void)kek_state_store_update_fields(store, app->binding.slots.player,
                                            player_pickup_update, &pickup,
                                            KEK_STATE_TYPE_PLAYER_FIELD_RAPID_TIMER);
    } else {
        (void)kek_state_store_update_fields(store, app->binding.slots.player,
                                            player_pickup_update, &pickup,
                                            KEK_STATE_TYPE_PLAYER_FIELD_SHIELD);
    }
    GameMode resume = GameMode_Playing;
    (void)kek_state_store_update_fields(store, app->binding.slots.session,
                                        set_mode_only_update, &resume,
                                        KEK_STATE_TYPE_GAME_SESSION_FIELD_MODE);
}

static void update_game(GameApp* app, float dt) {
    InputIntent input = collect_input(app);
    (void)kek_state_store_update_fields(game_store(app), app->binding.slots.input,
                                        input_update, &input,
                                        KEK_EVENT_CHANGED_FIELDS_UNKNOWN);

    const GameSession* session = session_const(app);
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
            (void)kek_state_store_update_fields(game_store(app), app->binding.slots.session,
                                                set_mode_only_update, &resume,
                                                KEK_STATE_TYPE_GAME_SESSION_FIELD_MODE);
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

    (void)kek_event_dispatch_pending(kek_runtime_events(&app->runtime));
}

int OnFrameTimer(KekHookContext* context) {
    GameApp* app = (GameApp*)context->app_context;
    const GameSession* session = game_state_session_const(context->state_store, &app->binding.slots);
    const WaveDirector* wave = game_state_wave_const(context->state_store, &app->binding.slots);
    if (!session || !wave || session->mode != GameMode_Playing) {
        return 1;
    }
    if (wave->spawn_budget == 0 &&
        count_slots(context->state_store, game_state_enemy_first, game_state_enemy_next) == 0) {
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
    KekStateStore* store = game_store(app);
    const Player* player = player_const(app);
    const InputIntent* input = game_state_input_const(store, &app->binding.slots);
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

    for (size_t slot = game_state_pickup_first(store); slot != KEK_STATE_INVALID_ID; slot = game_state_pickup_next(store, slot)) {
        const Pickup* pickup = game_state_pickup_slot_const(store, slot);
        if (!pickup) {
            continue;
        }
        float radius = PICKUP_RADIUS + sinf(pickup->pulse) * 2.0f;
        DrawCircleV((Vector2){pickup->x, pickup->y}, radius, pickup_color(pickup->kind));
        DrawCircleLines((int)pickup->x, (int)pickup->y, radius + 4.0f, Fade(WHITE, 0.35f));
    }

    for (size_t slot = game_state_projectile_first(store); slot != KEK_STATE_INVALID_ID; slot = game_state_projectile_next(store, slot)) {
        const Projectile* projectile = game_state_projectile_slot_const(store, slot);
        if (projectile) {
            DrawCircleV((Vector2){projectile->x, projectile->y}, PROJECTILE_RADIUS, SKYBLUE);
            DrawCircleV((Vector2){projectile->x, projectile->y}, PROJECTILE_RADIUS * 0.45f, WHITE);
        }
    }

    for (size_t slot = game_state_enemy_first(store); slot != KEK_STATE_INVALID_ID; slot = game_state_enemy_next(store, slot)) {
        const Enemy* enemy = game_state_enemy_slot_const(store, slot);
        if (!enemy) {
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
    KekStateStore* store = game_store(app);
    const GameSession* session = session_const(app);
    const Player* player = player_const(app);
    const WaveDirector* wave = wave_const(app);
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

    for (size_t slot = game_state_hud_message_first(store); slot != KEK_STATE_INVALID_ID; slot = game_state_hud_message_next(store, slot)) {
        const HudMessage* message = game_state_hud_message_slot_const(store, slot);
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
        DrawText(TextFormat("Enemies %d Projectiles %d", count_slots(store, game_state_enemy_first, game_state_enemy_next),
                            count_slots(store, game_state_projectile_first, game_state_projectile_next)),
                 SCREEN_WIDTH - 300, 54, 18, RAYWHITE);
        DrawText(TextFormat("Pickups %d HUD %d", count_slots(store, game_state_pickup_first, game_state_pickup_next),
                            count_slots(store, game_state_hud_message_first, game_state_hud_message_next)),
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
    const CameraRig* camera = game_state_camera_const(game_store(app), &app->binding.slots);
    const GameSession* session = session_const(app);
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
    const GameSession* session = session_const(app);
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

int main(void) {
    GameApp app;
    memset(&app, 0, sizeof(app));
    kek_runtime_init(&app.runtime);
    if (!game_state_runtime_binding_init(&app.binding, &app.runtime, &app)) {
        fprintf(stderr, "failed to initialize generated game state binding\n");
        kek_runtime_destroy(&app.runtime);
        return 1;
    }

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Kek generated-state raylib game");
    SetTargetFPS(60);

    while (!app.should_quit) {
        float dt = clampf(GetFrameTime(), 0.0f, 0.05f);
        update_game(&app, dt);
        draw_game(&app);
    }

    CloseWindow();
    game_state_runtime_binding_destroy(&app.binding);
    kek_runtime_destroy(&app.runtime);
    return 0;
}
