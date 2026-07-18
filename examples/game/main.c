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

typedef struct FrameClockUpdate {
    uint64_t tick;
    float dt;
} FrameClockUpdate;

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
    float speed;
    float strafe;
} EnemyUpdate;

typedef struct EnemyDamageUpdate {
    int damage;
} EnemyDamageUpdate;

typedef struct EnemyResetUpdate {
    Enemy enemy;
} EnemyResetUpdate;

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

static bool is_declared_enemy_slot(const GameApp* app, size_t slot) {
    return slot == app->binding.slots.grunt_enemy || slot == app->binding.slots.runner_enemy ||
           slot == app->binding.slots.tank_enemy || slot == app->binding.slots.boss_enemy;
}

static int count_active_enemies(KekStateStore* store) {
    int count = 0;
    for (size_t slot = game_state_enemy_first(store); slot != KEK_STATE_INVALID_ID; slot = game_state_enemy_next(store, slot)) {
        const Enemy* enemy = game_state_enemy_slot_const(store, slot);
        if (enemy && enemy->active) {
            count++;
        }
    }
    return count;
}

#include "game_logic.inc.c"
#include "game_hooks.inc.c"
#include "game_render.inc.c"

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
