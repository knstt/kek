#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "raylib.h"

#include "examples/game/game_app.h"

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

#include "game_logic.inc.c"
#include "game_render.inc.c"

int main(void) {
    GameApp app;
    memset(&app, 0, sizeof(app));
    if (!game_state_runtime_init(&app.state, &app)) {
        fprintf(stderr, "failed to initialize generated game state runtime\n");
        return 1;
    }
#ifdef KEK_HOOK_DYNAMIC
    if (!kek_hook_registry_load_library(&app.state.app.hook_registry, KEK_GAME_HOOK_LIBRARY)) {
        fprintf(stderr, "failed to load hook library %s\n", KEK_GAME_HOOK_LIBRARY);
        game_state_runtime_destroy(&app.state);
        return 1;
    }
#endif

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Kek generated-state raylib game");
    SetTargetFPS(60);

    while (!app.should_quit) {
#ifdef KEK_HOOK_DYNAMIC
        (void)kek_hook_registry_reload_library(&app.state.app.hook_registry);
#endif
        float dt = clampf(GetFrameTime(), 0.0f, 0.05f);
        update_game(&app, dt);
        draw_game(&app);
    }

    CloseWindow();
    game_state_runtime_destroy(&app.state);
    return 0;
}
