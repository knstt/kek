#ifndef KEK_EXAMPLES_GAME_APP_H
#define KEK_EXAMPLES_GAME_APP_H

#include <stdbool.h>
#include <stdint.h>

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
    Game_stateRuntime state;
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

void set_mode_update(void* draft, void* context);
void wave_full_update(void* draft, void* context);
void enemy_update(void* draft, void* context);

#endif
