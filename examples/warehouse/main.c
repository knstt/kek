#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include "generated/warehouse.h"
#include "runtime/hook.h"
#include "runtime/runtime.h"
#include "runtime/state_storage.h"
#include "runtime/stream.h"

#define WAREHOUSE_TEXT_CAPACITY 1001

typedef struct WarehouseApp {
    KekRuntime* runtime;
    KekStream* stdin_stream;
    KekStream* stdout_stream;
    KekStateStore state_store;
    KekHookRegistry hook_registry;
    size_t standard_input_slot;
    size_t standard_output_slot;
    size_t player_command_slot;
    size_t worker_slot;
    size_t warehouse_map_slot;
    size_t package_slot;
    size_t delivery_zone_slot;
    size_t game_status_slot;
    char input_buffer[WAREHOUSE_TEXT_CAPACITY];
    char output_buffer[WAREHOUSE_TEXT_CAPACITY];
    size_t input_len;
    size_t handled_input_len;
    size_t output_len;
    uint64_t ignored_worker_version;
} WarehouseApp;

typedef struct TextStateUpdate {
    char* data;
    size_t len;
} TextStateUpdate;

typedef struct StateCopyUpdate {
    const void* value;
    size_t size;
} StateCopyUpdate;

typedef struct CommandUpdate {
    int dx;
    int dy;
    int reset_requested;
    int quit_requested;
} CommandUpdate;

static const Worker* app_worker_const(const WarehouseApp* app) {
    return (const Worker*)kek_state_store_current_const(&app->state_store, app->worker_slot);
}

static const PlayerCommand* app_player_command_const(const WarehouseApp* app) {
    return (const PlayerCommand*)kek_state_store_current_const(&app->state_store,
                                                               app->player_command_slot);
}

static const WarehouseMap* app_warehouse_map_const(const WarehouseApp* app) {
    return (const WarehouseMap*)kek_state_store_current_const(&app->state_store,
                                                              app->warehouse_map_slot);
}

static const Package* app_package_const(const WarehouseApp* app) {
    return (const Package*)kek_state_store_current_const(&app->state_store, app->package_slot);
}

static const DeliveryZone* app_delivery_zone_const(const WarehouseApp* app) {
    return (const DeliveryZone*)kek_state_store_current_const(&app->state_store,
                                                             app->delivery_zone_slot);
}

static const GameStatus* app_game_status_const(const WarehouseApp* app) {
    return (const GameStatus*)kek_state_store_current_const(&app->state_store,
                                                           app->game_status_slot);
}

static const StandardInput* app_standard_input_const(const WarehouseApp* app) {
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

static void update_state_copy(void* draft, void* context) {
    StateCopyUpdate* update = (StateCopyUpdate*)context;
    memcpy(draft, update->value, update->size);
}

static void update_player_command(void* draft, void* context) {
    CommandUpdate* update = (CommandUpdate*)context;
    PlayerCommand* command = (PlayerCommand*)draft;
    command->dx = update->dx;
    command->dy = update->dy;
    command->reset_requested = update->reset_requested != 0;
    command->quit_requested = update->quit_requested != 0;
    command->sequence++;
}

static void update_reset_state(void* draft, void* context) {
    const KekStateDescriptor* descriptor = (const KekStateDescriptor*)context;
    if (descriptor && descriptor->reset) {
        descriptor->reset(draft);
    }
}

static int app_commit_state_copy(WarehouseApp* app, size_t slot_id, const void* value,
                                 size_t size) {
    StateCopyUpdate update = {value, size};
    return kek_state_store_update(&app->state_store, slot_id, update_state_copy, &update);
}

static void status_set_message(GameStatus* status, const char* message) {
    status->message = kek_string_from_cstr(message);
}

static void app_set_input_state(WarehouseApp* app, const char* data, size_t len) {
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

static void app_track_output_state(WarehouseApp* app, const char* data, size_t len) {
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

static void app_write_raw(WarehouseApp* app, const char* data, size_t len) {
    kek_stream_flush(app->stdout_stream);
    size_t written = kek_stream_write_raw(app->stdout_stream, data, len);
    if (written != len) {
        kek_stream_flush(app->stdout_stream);
        written += kek_stream_write_raw(app->stdout_stream, data + written, len - written);
    }
    if (written != len) {
        fprintf(stderr, "stdout stream buffer full, dropped %zu bytes\n", len - written);
    }
    app_track_output_state(app, data, written);
}

static void app_write(WarehouseApp* app, const char* text) {
    if (text) {
        app_write_raw(app, text, strlen(text));
    }
}

static void warehouse_reset(WarehouseApp* app) {
    size_t slots[] = {app->player_command_slot, app->worker_slot, app->warehouse_map_slot,
                      app->package_slot, app->delivery_zone_slot, app->game_status_slot};
    for (size_t i = 0; i < sizeof(slots) / sizeof(slots[0]); i++) {
        const KekStateDescriptor* descriptor = kek_state_store_descriptor(&app->state_store,
                                                                          slots[i]);
        kek_state_store_update(&app->state_store, slots[i], update_reset_state,
                               (void*)descriptor);
    }
}

static char warehouse_cell_at(const WarehouseApp* app, int x, int y) {
    const Worker* worker = app_worker_const(app);
    const Package* package = app_package_const(app);
    const DeliveryZone* zone = app_delivery_zone_const(app);

    if (worker->x == x && worker->y == y) {
        return worker->carrying ? 'W' : '@';
    }
    if (!package->delivered && !worker->carrying && package->package_x == x &&
        package->package_y == y) {
        return 'p';
    }
    if (zone->zone_x == x && zone->zone_y == y) {
        return 'D';
    }
    return '.';
}

static void warehouse_render(WarehouseApp* app) {
    const Worker* worker = app_worker_const(app);
    const WarehouseMap* map = app_warehouse_map_const(app);
    const GameStatus* status = app_game_status_const(app);
    KekString message = status->message;
    char screen[1000];
    size_t used = 0;

    int count = snprintf(screen + used, sizeof(screen) - used,
                         "\033[2J\033[H"
                         "Warehouse Picker - Kek state example\n"
                         "====================================\n"
                         "Goal: pick up 'p' and deliver it to 'D'.\n"
                         "Controls: W/A/S/D or arrow keys move, R restarts, Q quits.\n\n"
                         "Energy: %d | Score: %d | Turn: %d | Carrying: %s\n\n",
                         worker->energy, worker->score, status->turn,
                         worker->carrying ? "yes" : "no");
    if (count < 0) {
        return;
    }
    used += (size_t)count;

    for (int y = 0; y < map->height; y++) {
        for (int x = 0; x < map->width; x++) {
            char cell = '#';
            if (x > 0 && x < map->width - 1 && y > 0 && y < map->height - 1) {
                cell = warehouse_cell_at(app, x, y);
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
                     "\nLegend: @ worker, W carrying, p package, D delivery, # wall\n"
                     "Status: %.*s\n",
                     (int)message.len, message.data);
    if (count < 0) {
        return;
    }
    used += (size_t)count;

    if (status->game_over) {
        count = snprintf(screen + used, sizeof(screen) - used,
                         "\n%s Press R to restart or Q to quit.\n",
                         status->won ? "Delivery complete." : "Shift failed.");
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

static void warehouse_publish_command(WarehouseApp* app, int dx, int dy, int reset_requested,
                                      int quit_requested) {
    CommandUpdate update = {dx, dy, reset_requested, quit_requested};
    if (kek_state_store_update(&app->state_store, app->player_command_slot,
                               update_player_command, &update)) {
        kek_event_dispatch_pending(kek_runtime_events(app->runtime));
    }
}

void ApplyCommandChanged(KekHookContext* context) {
    WarehouseApp* app = (WarehouseApp*)context->app_context;
    if (!app) {
        return;
    }

    const PlayerCommand* command = app_player_command_const(app);
    if (!command) {
        return;
    }

    if (command->quit_requested) {
        app_write(app, "\033[2J\033[HWarehouse shift ended.\n");
        kek_runtime_request_quit(app->runtime);
        return;
    }

    if (command->reset_requested) {
        warehouse_reset(app);
        return;
    }

    if (command->dx == 0 && command->dy == 0) {
        GameStatus status = *app_game_status_const(app);
        status_set_message(&status, "Use W/A/S/D, arrow keys, R, or Q.");
        app_commit_state_copy(app, app->game_status_slot, &status, sizeof(status));
        return;
    }

    Worker worker = *app_worker_const(app);
    const WarehouseMap* map = app_warehouse_map_const(app);
    GameStatus status = *app_game_status_const(app);

    if (status.game_over) {
        status_set_message(&status, "The shift is over. Press R to restart or Q to quit.");
        app_commit_state_copy(app, app->game_status_slot, &status, sizeof(status));
        return;
    }

    int next_x = worker.x + command->dx;
    int next_y = worker.y + command->dy;
    if (next_x < 1 || next_x > map->width - 2 || next_y < 1 || next_y > map->height - 2) {
        status_set_message(&status, "A shelf blocks that path.");
        app_commit_state_copy(app, app->game_status_slot, &status, sizeof(status));
        return;
    }

    worker.x = next_x;
    worker.y = next_y;
    worker.energy--;

    if (!Worker_check(&worker)) {
        status_set_message(&status, "The requested move would put the worker in an invalid state.");
        app_commit_state_copy(app, app->game_status_slot, &status, sizeof(status));
        return;
    }

    app_commit_state_copy(app, app->worker_slot, &worker, sizeof(worker));
}

void UpdatePackageAfterWorkerChanged(KekHookContext* context) {
    WarehouseApp* app = (WarehouseApp*)context->app_context;
    if (!app || !context->event || context->event->state_slot_id != app->worker_slot) {
        return;
    }
    if (context->event->state_version == app->ignored_worker_version) {
        app->ignored_worker_version = 0;
        return;
    }

    Worker worker = *app_worker_const(app);
    Package package = *app_package_const(app);
    const DeliveryZone* zone = app_delivery_zone_const(app);
    GameStatus status = *app_game_status_const(app);

    if (status.game_over) {
        return;
    }

    status.turn++;
    status_set_message(&status, "You hurry through the warehouse aisles.");
    int worker_changed = 0;
    int package_changed = 0;

    if (!package.delivered && !worker.carrying && worker.x == package.package_x &&
        worker.y == package.package_y) {
        worker.carrying = true;
        worker.score += 10;
        worker_changed = 1;
        status_set_message(&status, "Package picked. Deliver it to the marked zone.");
    }

    if (worker.carrying && worker.x == zone->zone_x && worker.y == zone->zone_y) {
        worker.carrying = false;
        package.delivered = true;
        worker.score += 90;
        worker_changed = 1;
        package_changed = 1;
        status_set_message(&status, "Package dropped at the delivery zone.");
    } else if (worker.energy == 0) {
        status.game_over = true;
        status.won = false;
        status_set_message(&status, "Energy depleted before delivery.");
    }

    if (!Worker_check(&worker) || !Package_check(&package) || !GameStatus_check(&status)) {
        return;
    }

    if (worker_changed && app_commit_state_copy(app, app->worker_slot, &worker, sizeof(worker))) {
        app->ignored_worker_version = kek_state_store_version(&app->state_store,
                                                              app->worker_slot);
    }
    if (package_changed) {
        app_commit_state_copy(app, app->package_slot, &package, sizeof(package));
    }
    app_commit_state_copy(app, app->game_status_slot, &status, sizeof(status));
}

void UpdateStatusAfterPackageChanged(KekHookContext* context) {
    WarehouseApp* app = (WarehouseApp*)context->app_context;
    if (!app || !context->event || context->event->state_slot_id != app->package_slot) {
        return;
    }

    const Package* package = app_package_const(app);
    const Worker* worker = app_worker_const(app);
    GameStatus status = *app_game_status_const(app);

    if (package->delivered && !status.game_over) {
        status.game_over = true;
        status.won = true;
        status_set_message(&status, "Package delivered on time.");
        app_commit_state_copy(app, app->game_status_slot, &status, sizeof(status));
    } else if (worker->energy == 0 && !status.game_over) {
        status.game_over = true;
        status.won = false;
        status_set_message(&status, "Energy depleted before delivery.");
        app_commit_state_copy(app, app->game_status_slot, &status, sizeof(status));
    }
}

void RenderAfterStatusChanged(KekHookContext* context) {
    WarehouseApp* app = (WarehouseApp*)context->app_context;
    if (app) {
        warehouse_render(app);
    }
}

static void warehouse_handle_arrow(WarehouseApp* app, char arrow);

static void warehouse_handle_key(WarehouseApp* app, char key) {
    switch (key) {
        case 'w':
        case 'W':
            warehouse_publish_command(app, 0, -1, 0, 0);
            break;
        case 'a':
        case 'A':
            warehouse_publish_command(app, -1, 0, 0, 0);
            break;
        case 's':
        case 'S':
            warehouse_publish_command(app, 0, 1, 0, 0);
            break;
        case 'd':
        case 'D':
            warehouse_publish_command(app, 1, 0, 0, 0);
            break;
        case 'r':
        case 'R':
            warehouse_publish_command(app, 0, 0, 1, 0);
            break;
        case 'q':
        case 'Q':
            warehouse_publish_command(app, 0, 0, 0, 1);
            break;
        default: {
            warehouse_publish_command(app, 0, 0, 0, 0);
            break;
        }
    }
}

static void warehouse_handle_arrow(WarehouseApp* app, char arrow) {
    switch (arrow) {
        case 'A':
            warehouse_publish_command(app, 0, -1, 0, 0);
            break;
        case 'B':
            warehouse_publish_command(app, 0, 1, 0, 0);
            break;
        case 'C':
            warehouse_publish_command(app, 1, 0, 0, 0);
            break;
        case 'D':
            warehouse_publish_command(app, -1, 0, 0, 0);
            break;
        default:
            warehouse_handle_key(app, arrow);
            break;
    }
}

void HandleInputChanged(KekHookContext* context) {
    WarehouseApp* app = (WarehouseApp*)context->app_context;
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
            warehouse_handle_arrow(app, data[i + 2]);
            i += 2;
        } else if (ch != '\n' && ch != '\r') {
            warehouse_handle_key(app, ch);
        }
    }
}

static void stream_data_handler(const KekEvent* event, void* context) {
    WarehouseApp* app = (WarehouseApp*)context;
    if (event->source != app->stdin_stream || event->data_len == 0) {
        return;
    }
    app_set_input_state(app, event->data, event->data_len);
}

static void stream_error_handler(const KekEvent* event, void* context) {
    WarehouseApp* app = (WarehouseApp*)context;
    if (event->source == app->stdin_stream) {
        fprintf(stderr, "input stream error: %d\n", event->error_code);
        kek_runtime_request_quit(app->runtime);
    }
}

static int app_add_states(WarehouseApp* app) {
    size_t slots[KEK_STATE_TYPE_COUNT];
    if (!kek_generated_state_store_add_defaults(&app->state_store, slots)) {
        return 0;
    }

    app->standard_input_slot = slots[KEK_STATE_TYPE_STANDARD_INPUT];
    app->standard_output_slot = slots[KEK_STATE_TYPE_STANDARD_OUTPUT];
    app->player_command_slot = slots[KEK_STATE_TYPE_PLAYER_COMMAND];
    app->worker_slot = slots[KEK_STATE_TYPE_WORKER];
    app->warehouse_map_slot = slots[KEK_STATE_TYPE_WAREHOUSE_MAP];
    app->package_slot = slots[KEK_STATE_TYPE_PACKAGE];
    app->delivery_zone_slot = slots[KEK_STATE_TYPE_DELIVERY_ZONE];
    app->game_status_slot = slots[KEK_STATE_TYPE_GAME_STATUS];

    return 1;
}

static int app_init(WarehouseApp* app, KekRuntime* runtime, KekStream* stdin_stream,
                    KekStream* stdout_stream) {
    memset(app, 0, sizeof(*app));
    app->runtime = runtime;
    app->stdin_stream = stdin_stream;
    app->stdout_stream = stdout_stream;

    kek_state_store_init(&app->state_store, runtime);
    if (!app_add_states(app)) {
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
    if (stdin_id < 0 || stdout_id < 0) {
        fprintf(stderr, "failed to register standard stream states\n");
        kek_runtime_disable_raw_mode(&runtime, STDIN_FILENO);
        kek_runtime_destroy(&runtime);
        return 1;
    }

    WarehouseApp app;
    if (app_init(&app, &runtime, kek_runtime_get_stream(&runtime, (size_t)stdin_id),
                 kek_runtime_get_stream(&runtime, (size_t)stdout_id)) < 0) {
        fprintf(stderr, "failed to initialize warehouse state storage\n");
        kek_runtime_disable_raw_mode(&runtime, STDIN_FILENO);
        kek_runtime_destroy(&runtime);
        return 1;
    }

    kek_event_subscribe(kek_runtime_events(&runtime), KEK_EVENT_STREAM_DATA,
                        stream_data_handler, &app);
    kek_event_subscribe(kek_runtime_events(&runtime), KEK_EVENT_STREAM_ERROR,
                        stream_error_handler, &app);
    warehouse_render(&app);

    int result = kek_runtime_run(&runtime);
    kek_runtime_disable_raw_mode(&runtime, STDIN_FILENO);
    kek_hook_registry_detach(&app.hook_registry);
    kek_state_store_destroy(&app.state_store);
    kek_runtime_destroy(&runtime);
    return result == 0 ? 0 : 1;
}
