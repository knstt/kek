#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "generated/game.h"
#include "runtime/runtime.h"
#include "runtime/hook.h"
#include "runtime/state_storage.h"
#include "runtime/stream.h"

#define GAME_TEXT_CAPACITY 1001

typedef struct GameApp {
    KekRuntime* runtime;
    KekStream* stdin_stream;
    KekStream* stdout_stream;
    KekStream* log_stream;
    KekStateStore state_store;
    KekHookRegistry hook_registry;
    size_t standard_input_slot;
    size_t standard_output_slot;
    size_t player_slot;
    size_t dungeon_map_slot;
    size_t treasure_slot;
    size_t goblin_slot;
    size_t game_progress_slot;
    char input_buffer[GAME_TEXT_CAPACITY];
    char output_buffer[GAME_TEXT_CAPACITY];
    size_t input_len;
    size_t handled_input_len;
    size_t output_len;
} GameApp;

static void game_publish_state(GameApp* app);
static void game_render(GameApp* app);
static void game_handle_key(GameApp* app, char key);
static void game_handle_arrow(GameApp* app, char arrow);

typedef struct TextStateUpdate {
    char* data;
    size_t len;
} TextStateUpdate;

typedef struct MoveUpdate {
    int dx;
    int dy;
} MoveUpdate;

static Player* app_player(GameApp* app) {
    return (Player*)kek_state_store_current(&app->state_store, app->player_slot);
}

static const Player* app_player_const(const GameApp* app) {
    return (const Player*)kek_state_store_current_const(&app->state_store, app->player_slot);
}

static const DungeonMap* app_dungeon_map_const(const GameApp* app) {
    return (const DungeonMap*)kek_state_store_current_const(&app->state_store, app->dungeon_map_slot);
}

static const Treasure* app_treasure_const(const GameApp* app) {
    return (const Treasure*)kek_state_store_current_const(&app->state_store, app->treasure_slot);
}

static const Goblin* app_goblin_const(const GameApp* app) {
    return (const Goblin*)kek_state_store_current_const(&app->state_store, app->goblin_slot);
}

static const GameProgress* app_game_progress_const(const GameApp* app) {
    return (const GameProgress*)kek_state_store_current_const(&app->state_store,
                                                              app->game_progress_slot);
}

static const StandardInput* app_standard_input_const(const GameApp* app) {
    return (const StandardInput*)kek_state_store_current_const(&app->state_store,
                                                               app->standard_input_slot);
}

static void update_standard_input(void* draft, void* context) {
    TextStateUpdate* text = (TextStateUpdate*)context;
    StandardInput* state = (StandardInput*)draft;
    state->input = (KekString){text->data, text->len};
}

static void update_standard_output(void* draft, void* context) {
    TextStateUpdate* text = (TextStateUpdate*)context;
    StandardOutput* state = (StandardOutput*)draft;
    state->output = (KekString){text->data, text->len};
}

static void game_set_message_on_state(GameProgress* progress, const char* message) {
    progress->message = kek_string_from_cstr(message);
}

static void app_set_input_state(GameApp* app, const char* data, size_t len) {
    size_t available = sizeof(app->input_buffer) - app->input_len - 1;
    size_t to_copy = len < available ? len : available;
    if (to_copy > 0) {
        memcpy(app->input_buffer + app->input_len, data, to_copy);
        app->input_len += to_copy;
        app->input_buffer[app->input_len] = '\0';
    }

    TextStateUpdate update = {app->input_buffer, app->input_len};
    kek_state_store_update(&app->state_store, app->standard_input_slot,
                           update_standard_input, &update);
}

static void app_track_output_state(GameApp* app, const char* data, size_t len) {
    size_t available = sizeof(app->output_buffer) - app->output_len - 1;
    if (available == 0) {
        app->output_len = 0;
        app->output_buffer[0] = '\0';
        available = sizeof(app->output_buffer) - 1;
    }
    size_t to_copy = len < available ? len : available;
    if (to_copy > 0) {
        memcpy(app->output_buffer + app->output_len, data, to_copy);
        app->output_len += to_copy;
        app->output_buffer[app->output_len] = '\0';
    }

    TextStateUpdate update = {app->output_buffer, app->output_len};
    kek_state_store_update(&app->state_store, app->standard_output_slot,
                           update_standard_output, &update);
}

static void app_write_raw(GameApp* app, const char* data, size_t len) {
    size_t written = kek_stream_write_raw(app->stdout_stream, data, len);
    if (written != len) {
        fprintf(stderr, "stdout stream buffer full, dropped %zu bytes\n", len - written);
    }
    app_track_output_state(app, data, written);
}

static void app_write(GameApp* app, const char* text) {
    if (text) {
        app_write_raw(app, text, strlen(text));
    }
}

static void game_publish_state(GameApp* app) {
    kek_runtime_publish_state_slot_changed(app->runtime, app_player(app),
                                           KEK_STATE_TYPE_PLAYER, app->player_slot,
                                           kek_state_store_version(&app->state_store,
                                                                   app->player_slot));
}

static void update_game_message(void* draft, void* context) {
    game_set_message_on_state((GameProgress*)draft, (const char*)context);
}

static void game_set_message(GameApp* app, const char* message) {
    kek_state_store_update(&app->state_store, app->game_progress_slot,
                           update_game_message, (void*)message);
}

static void update_reset_state(void* draft, void* context) {
    const KekStateDescriptor* descriptor = (const KekStateDescriptor*)context;
    if (descriptor && descriptor->reset) {
        descriptor->reset(draft);
    }
}

static void game_reset(GameApp* app) {
    size_t slots[] = {app->player_slot, app->dungeon_map_slot, app->treasure_slot,
                      app->goblin_slot, app->game_progress_slot};
    for (size_t i = 0; i < sizeof(slots) / sizeof(slots[0]); i++) {
        const KekStateDescriptor* descriptor = kek_state_store_descriptor(&app->state_store,
                                                                          slots[i]);
        kek_state_store_update(&app->state_store, slots[i], update_reset_state,
                               (void*)descriptor);
    }
}

static char game_cell_at(const GameApp* app, int x, int y) {
    const Player* player = app_player_const(app);
    const Treasure* treasure = app_treasure_const(app);
    const Goblin* goblin = app_goblin_const(app);
    const GameProgress* progress = app_game_progress_const(app);
    int player_x = player->x;
    int player_y = player->y;

    if (player_x == x && player_y == y) {
        return '@';
    }
    if (!player->has_treasure && treasure->treasure_x == x && treasure->treasure_y == y) {
        return '$';
    }
    if (goblin->goblin_alive && goblin->goblin_x == x && goblin->goblin_y == y) {
        return 'g';
    }
    if (progress->exit_x == x && progress->exit_y == y) {
        return '>';
    }
    return '.';
}

static void game_render(GameApp* app) {
    const Player* player = app_player_const(app);
    const DungeonMap* map = app_dungeon_map_const(app);
    const GameProgress* progress = app_game_progress_const(app);
    KekString name = player->name;
    KekString message = progress->message;
    char screen[2048];
    size_t used = 0;

    int count = snprintf(screen + used, sizeof(screen) - used,
                         "\033[2J\033[H"
                         "Tiny Dungeon - Kek state example\n"
                         "================================\n"
                          "Goal: collect '$', then reach '>'. Avoid or defeat 'g'.\n"
                         "Controls: W/A/S/D or arrow keys move, R restarts, Q quits.\n\n"
                         "Hero: %.*s | Health: %d | Gold: %d | Turn: %d | Treasure: %s\n\n",
                         (int)name.len, name.data, player->health, player->gold,
                         progress->turn, player->has_treasure ? "yes" : "no");
    if (count < 0) {
        return;
    }
    used += (size_t)count;

    int map_width = map->width;
    int map_height = map->height;
    for (int y = 0; y < map_height; y++) {
        for (int x = 0; x < map_width; x++) {
            char cell = '#';
            if (x > 0 && x < map_width - 1 && y > 0 && y < map_height - 1) {
                cell = game_cell_at(app, x, y);
            }
            if (used + 2 < sizeof(screen)) {
                screen[used++] = cell;
            }
        }
        if (used + 2 < sizeof(screen)) {
            screen[used++] = '\n';
        }
    }

    count = snprintf(screen + used, sizeof(screen) - used,
                     "\nLegend: @ you, $ treasure, g goblin, > exit, # wall\n"
                     "Status: %.*s\n",
                     (int)message.len, message.data);
    if (count < 0) {
        return;
    }
    used += (size_t)count;

    if (progress->game_over) {
        count = snprintf(screen + used, sizeof(screen) - used,
                         "\n%s Press R to play again or Q to quit.\n",
                         progress->won ? "You escaped with the treasure!"
                                       : "Your adventure is over.");
        if (count < 0) {
            return;
        }
        used += (size_t)count;
    }

    if (used >= sizeof(screen)) {
        used = sizeof(screen) - 1;
    }
    app_write_raw(app, screen, used);
}

static int game_state_is_next_to_goblin(const Player* player, const Goblin* goblin) {
    int dx = player->x - goblin->goblin_x;
    int dy = player->y - goblin->goblin_y;
    if (dx < 0) {
        dx = -dx;
    }
    if (dy < 0) {
        dy = -dy;
    }
    return goblin->goblin_alive && dx + dy == 1;
}

static void game_damage_player(Player* player, int amount) {
    int health = player->health - amount;
    if (health < 0) {
        health = 0;
    }
    player->health = health;
}

static void game_finish_if_health_empty(Player* player, GameProgress* progress) {
    if (player->health <= 0) {
        player->health = 0;
        progress->game_over = true;
        progress->won = false;
        game_set_message_on_state(progress, "The goblin got the better of you.");
    }
}

typedef struct StateCopyUpdate {
    const void* value;
    size_t size;
} StateCopyUpdate;

static void update_state_copy(void* draft, void* context) {
    StateCopyUpdate* update = (StateCopyUpdate*)context;
    memcpy(draft, update->value, update->size);
}

static int app_commit_state_copy(GameApp* app, size_t slot_id, const void* value,
                                 size_t size) {
    StateCopyUpdate update = {value, size};
    return kek_state_store_update(&app->state_store, slot_id, update_state_copy, &update);
}

static int game_apply_move(GameApp* app, const MoveUpdate* move) {
    Player player = *app_player_const(app);
    const DungeonMap* map = app_dungeon_map_const(app);
    const Treasure* treasure = app_treasure_const(app);
    Goblin goblin = *app_goblin_const(app);
    GameProgress progress = *app_game_progress_const(app);

    if (progress.game_over) {
        game_set_message_on_state(&progress, "The game is finished. Press R to restart or Q to quit.");
        return app_commit_state_copy(app, app->game_progress_slot, &progress, sizeof(progress));
    }

    int next_x = player.x + move->dx;
    int next_y = player.y + move->dy;
    if (next_x < 1 || next_x > map->width - 2 || next_y < 1 ||
        next_y > map->height - 2) {
        game_set_message_on_state(&progress, "A stone wall blocks that path.");
        return app_commit_state_copy(app, app->game_progress_slot, &progress, sizeof(progress));
    }

    player.x = next_x;
    player.y = next_y;
    progress.turn++;
    game_set_message_on_state(&progress, "You move carefully through the dungeon.");

    if (!player.has_treasure && player.x == treasure->treasure_x &&
        player.y == treasure->treasure_y) {
        player.has_treasure = true;
        player.gold += 10;
        game_set_message_on_state(&progress, "You found the treasure. Now get to the exit!");
    }

    if (goblin.goblin_alive && player.x == goblin.goblin_x &&
        player.y == goblin.goblin_y) {
        goblin.goblin_alive = false;
        game_damage_player(&player, 35);
        player.gold += 3;
        game_set_message_on_state(&progress, "You defeat the goblin, but it lands a heavy hit.");
    } else if (game_state_is_next_to_goblin(&player, &goblin)) {
        game_damage_player(&player, 10);
        game_set_message_on_state(&progress, "The goblin swipes at you from the shadows.");
    }

    if (player.x == progress.exit_x && player.y == progress.exit_y) {
        if (player.has_treasure) {
            progress.game_over = true;
            progress.won = true;
            game_set_message_on_state(&progress, "You step into daylight with the treasure secured.");
        } else {
            game_set_message_on_state(&progress, "The exit is here, but you still need the treasure.");
        }
    }

    game_finish_if_health_empty(&player, &progress);

    if (!Player_check(&player) || !Goblin_check(&goblin) ||
        !GameProgress_check(&progress)) {
        return 0;
    }

    int ok = app_commit_state_copy(app, app->player_slot, &player, sizeof(player));
    ok = app_commit_state_copy(app, app->goblin_slot, &goblin, sizeof(goblin)) && ok;
    ok = app_commit_state_copy(app, app->game_progress_slot, &progress, sizeof(progress)) && ok;
    return ok;
}

static void game_move(GameApp* app, int dx, int dy) {
    MoveUpdate move = {dx, dy};
    game_apply_move(app, &move);
}

void RenderAfterProgressChanged(KekHookContext* context) {
    GameApp* app = (GameApp*)context->app_context;
    if (app) {
        game_render(app);
    }
}

void HandleInputChanged(KekHookContext* context) {
    GameApp* app = (GameApp*)context->app_context;
    if (!app) {
        return;
    }

    const StandardInput* input = app_standard_input_const(app);
    if (!input || app->handled_input_len > input->input.len) {
        app->handled_input_len = input ? input->input.len : 0;
        return;
    }

    const char* data = input->input.data + app->handled_input_len;
    size_t len = input->input.len - app->handled_input_len;
    app->handled_input_len = input->input.len;

    for (size_t i = 0; i < len; i++) {
        char ch = data[i];
        if (ch == '\x1b' && i + 2 < len && data[i + 1] == '[') {
            game_handle_arrow(app, data[i + 2]);
            i += 2;
        } else if (ch != '\n' && ch != '\r') {
            game_handle_key(app, ch);
        }
    }
}

static void game_handle_key(GameApp* app, char key) {
    switch (key) {
        case 'w':
        case 'W':
            game_move(app, 0, -1);
            break;
        case 'a':
        case 'A':
            game_move(app, -1, 0);
            break;
        case 's':
        case 'S':
            game_move(app, 0, 1);
            break;
        case 'd':
        case 'D':
            game_move(app, 1, 0);
            break;
        case 'r':
        case 'R':
            game_reset(app);
            break;
        case 'q':
        case 'Q':
            game_set_message(app, "Quitting. Thanks for playing.");
            app_write(app, "\033[2J\033[HQuitting Tiny Dungeon.\n");
            kek_runtime_request_quit(app->runtime);
            break;
        default:
            game_set_message(app, "Use W/A/S/D, arrow keys, R, or Q.");
            break;
    }
}

static void game_handle_arrow(GameApp* app, char arrow) {
    switch (arrow) {
        case 'A':
            game_move(app, 0, -1);
            break;
        case 'B':
            game_move(app, 0, 1);
            break;
        case 'C':
            game_move(app, 1, 0);
            break;
        case 'D':
            game_move(app, -1, 0);
            break;
        default:
            game_handle_key(app, arrow);
            break;
    }
}

static void stream_data_handler(const KekEvent* event, void* context) {
    GameApp* app = (GameApp*)context;
    if (event->source != app->stdin_stream || event->data_len == 0) {
        return;
    }

    app_set_input_state(app, event->data, event->data_len);
    size_t logged = kek_stream_write_raw(app->log_stream, event->data, event->data_len);
    if (logged != event->data_len) {
        fprintf(stderr, "log stream buffer full, dropped %zu bytes\n", event->data_len - logged);
    }

}

static void stream_error_handler(const KekEvent* event, void* context) {
    GameApp* app = (GameApp*)context;
    if (event->source == app->stdin_stream) {
        fprintf(stderr, "input stream error: %d\n", event->error_code);
        kek_runtime_request_quit(app->runtime);
    }
}

static int app_init(GameApp* app, KekRuntime* runtime, KekStream* stdin_stream,
                    KekStream* stdout_stream, KekStream* log_stream) {
    memset(app, 0, sizeof(*app));
    app->runtime = runtime;
    app->stdin_stream = stdin_stream;
    app->stdout_stream = stdout_stream;
    app->log_stream = log_stream;
    kek_state_store_init(&app->state_store, runtime);
    app->standard_input_slot = kek_state_store_add(&app->state_store,
                                                   kek_generated_state_descriptor(KEK_STATE_TYPE_STANDARDINPUT),
                                                   NULL);
    app->standard_output_slot = kek_state_store_add(&app->state_store,
                                                    kek_generated_state_descriptor(KEK_STATE_TYPE_STANDARDOUTPUT),
                                                    NULL);
    app->player_slot = kek_state_store_add(&app->state_store,
                                           kek_generated_state_descriptor(KEK_STATE_TYPE_PLAYER),
                                           NULL);
    app->dungeon_map_slot = kek_state_store_add(&app->state_store,
                                                kek_generated_state_descriptor(KEK_STATE_TYPE_DUNGEONMAP),
                                                NULL);
    app->treasure_slot = kek_state_store_add(&app->state_store,
                                             kek_generated_state_descriptor(KEK_STATE_TYPE_TREASURE),
                                             NULL);
    app->goblin_slot = kek_state_store_add(&app->state_store,
                                           kek_generated_state_descriptor(KEK_STATE_TYPE_GOBLIN),
                                           NULL);
    app->game_progress_slot = kek_state_store_add(&app->state_store,
                                                  kek_generated_state_descriptor(KEK_STATE_TYPE_GAMEPROGRESS),
                                                  NULL);
    if (app->standard_input_slot == KEK_STATE_INVALID_ID ||
        app->standard_output_slot == KEK_STATE_INVALID_ID ||
        app->player_slot == KEK_STATE_INVALID_ID ||
        app->dungeon_map_slot == KEK_STATE_INVALID_ID ||
        app->treasure_slot == KEK_STATE_INVALID_ID ||
        app->goblin_slot == KEK_STATE_INVALID_ID ||
        app->game_progress_slot == KEK_STATE_INVALID_ID) {
        kek_state_store_destroy(&app->state_store);
        return -1;
    }
    kek_hook_registry_init(&app->hook_registry, runtime, &app->state_store, app);
    if (!kek_hook_registry_add_many(&app->hook_registry, KekGeneratedHookDescriptors,
                                    KEK_GENERATED_HOOK_COUNT)) {
        kek_state_store_destroy(&app->state_store);
        return -1;
    }
    kek_hook_registry_attach(&app->hook_registry);
    return 0;
}

int main(void) {
    KekRuntime runtime;
    kek_runtime_init(&runtime);
    if (kek_runtime_enable_raw_mode(&runtime, STDIN_FILENO) < 0) {
        kek_runtime_destroy(&runtime);
        return 1;
    }

    int stdin_id = kek_runtime_register_stream(&runtime, STDIN_FILENO, KEK_STREAM_READ, 0);
    int stdout_id = kek_runtime_register_stream(&runtime, STDOUT_FILENO, KEK_STREAM_WRITE, 0);

    int log_fd = open("keyboard_log.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (log_fd == -1) {
        perror("keyboard_log.txt");
        kek_runtime_disable_raw_mode(&runtime, STDIN_FILENO);
        kek_runtime_destroy(&runtime);
        return 1;
    }
    int log_id = kek_runtime_register_stream(&runtime, log_fd, KEK_STREAM_WRITE, 1);

    if (stdin_id < 0 || stdout_id < 0 || log_id < 0) {
        fprintf(stderr, "failed to register standard stream states\n");
        if (log_id < 0) {
            close(log_fd);
        }
        kek_runtime_disable_raw_mode(&runtime, STDIN_FILENO);
        kek_runtime_destroy(&runtime);
        return 1;
    }

    GameApp app;
    if (app_init(&app, &runtime, kek_runtime_get_stream(&runtime, (size_t)stdin_id),
                 kek_runtime_get_stream(&runtime, (size_t)stdout_id),
                 kek_runtime_get_stream(&runtime, (size_t)log_id)) < 0) {
        fprintf(stderr, "failed to initialize game state storage\n");
        kek_runtime_disable_raw_mode(&runtime, STDIN_FILENO);
        kek_runtime_destroy(&runtime);
        return 1;
    }

    kek_event_subscribe(kek_runtime_events(&runtime), KEK_EVENT_STREAM_DATA,
                        stream_data_handler, &app);
    kek_event_subscribe(kek_runtime_events(&runtime), KEK_EVENT_STREAM_ERROR,
                        stream_error_handler, &app);
    game_publish_state(&app);
    game_render(&app);

    int result = kek_runtime_run(&runtime);
    kek_runtime_disable_raw_mode(&runtime, STDIN_FILENO);
    kek_hook_registry_detach(&app.hook_registry);
    kek_state_store_destroy(&app.state_store);
    kek_runtime_destroy(&runtime);

    printf("Keyboard log saved to keyboard_log.txt\n");
    return result == 0 ? 0 : 1;
}
