#include "examples/game/game_app.h"

#include <math.h>

static float game_clampf(float value, float minimum, float maximum) {
    if (value < minimum) {
        return minimum;
    }
    if (value > maximum) {
        return maximum;
    }
    return value;
}

static float game_vec_len(float x, float y) {
    return sqrtf(x * x + y * y);
}

static void game_normalize_or(float* x, float* y, float fallback_x, float fallback_y) {
    float length = game_vec_len(*x, *y);
    if (length > 0.0001f) {
        *x /= length;
        *y /= length;
        return;
    }
    *x = fallback_x;
    *y = fallback_y;
}

void set_mode_update(void* draft, void* context) {
    GameSession* session = (GameSession*)draft;
    const ModeUpdate* update = (const ModeUpdate*)context;
    session->mode = update->mode;
    session->next_upgrade_score = update->next_upgrade_score;
}

void wave_full_update(void* draft, void* context) {
    WaveDirector* wave = (WaveDirector*)draft;
    const WaveFullUpdate* update = (const WaveFullUpdate*)context;
    *wave = update->wave;
}

void enemy_update(void* draft, void* context) {
    Enemy* enemy = (Enemy*)draft;
    const EnemyUpdate* update = (const EnemyUpdate*)context;
    if (!enemy->active) {
        return;
    }
    float dir_x = update->player.x - enemy->x;
    float dir_y = update->player.y - enemy->y;
    game_normalize_or(&dir_x, &dir_y, 0.0f, 0.0f);

    float move_x = dir_x - dir_y * update->strafe;
    float move_y = dir_y + dir_x * update->strafe;
    game_normalize_or(&move_x, &move_y, dir_x, dir_y);

    enemy->vx = move_x * update->speed;
    enemy->vy = move_y * update->speed;
    enemy->x = game_clampf(enemy->x + enemy->vx * update->dt,
                           -ARENA_HALF_WIDTH - 160.0f,
                           ARENA_HALF_WIDTH + 160.0f);
    enemy->y = game_clampf(enemy->y + enemy->vy * update->dt,
                           -ARENA_HALF_HEIGHT - 160.0f,
                           ARENA_HALF_HEIGHT + 160.0f);
    enemy->flash = game_clampf(enemy->flash - update->dt * 4.5f, 0.0f, 1.0f);
}
