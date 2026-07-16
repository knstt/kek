#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include "generated/warehouse.h"
#include "runtime/hook.h"
#include "runtime/runtime.h"
#include "runtime/standard_io.h"
#include "runtime/state_storage.h"
#include "runtime/stream.h"

#define WAREHOUSE_PACKAGE_COUNT 2

typedef struct WarehouseApp {
    KekRuntime* runtime;
    KekStream* stdin_stream;
    KekStream* stdout_stream;
    WarehouseRuntimeBinding binding;
    KekStandardTextBridge input_bridge;
    size_t handled_input_len;
    KekStandardTextBridge output_bridge;
} WarehouseApp;

#define APP_STORE(app) (&(app)->binding.state_store)
#define APP_SLOTS(app) (&(app)->binding.slots)

typedef struct StateCopyUpdate {
    const void* value;
    size_t size;
} StateCopyUpdate;

typedef struct CommandUpdate {
    int dx;
    int dy;
    int reset_requested;
    int quit_requested;
    Direction direction;
} CommandUpdate;

static size_t app_package_slot(const WarehouseApp* app, size_t index) {
    const size_t slots[WAREHOUSE_PACKAGE_COUNT] = {
        app->binding.slots.package_a,
        app->binding.slots.package_b,
    };
    return index < WAREHOUSE_PACKAGE_COUNT ? slots[index] : KEK_STATE_INVALID_ID;
}

static size_t app_delivery_zone_slot(const WarehouseApp* app, size_t index) {
    const size_t slots[WAREHOUSE_PACKAGE_COUNT] = {
        app->binding.slots.delivery_zone_a,
        app->binding.slots.delivery_zone_b,
    };
    return index < WAREHOUSE_PACKAGE_COUNT ? slots[index] : KEK_STATE_INVALID_ID;
}

static const Worker* app_worker_const(const WarehouseApp* app) {
    return warehouse_worker_const(APP_STORE(app), APP_SLOTS(app));
}

static const PlayerCommand* app_player_command_const(const WarehouseApp* app) {
    return warehouse_player_command_const(APP_STORE(app), APP_SLOTS(app));
}

static const WarehouseMap* app_warehouse_map_const(const WarehouseApp* app) {
    return warehouse_warehouse_map_const(APP_STORE(app), APP_SLOTS(app));
}

static const Package* app_package_const(const WarehouseApp* app, size_t index) {
    return warehouse_package_slot_const(APP_STORE(app), app_package_slot(app, index));
}

static const DeliveryZone* app_delivery_zone_const(const WarehouseApp* app, size_t index) {
    return warehouse_delivery_zone_slot_const(APP_STORE(app), app_delivery_zone_slot(app, index));
}

static const GameStatus* app_game_status_const(const WarehouseApp* app) {
    return warehouse_game_status_const(APP_STORE(app), APP_SLOTS(app));
}

static const StandardInput* app_standard_input_const(const WarehouseApp* app) {
    return warehouse_standard_input_const(APP_STORE(app), APP_SLOTS(app));
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
    command->recent_directions[0] = command->recent_directions[1];
    command->recent_directions[1] = command->recent_directions[2];
    command->recent_directions[2] = command->recent_directions[3];
    command->recent_directions[3] = update->direction;
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
    return kek_state_store_update(APP_STORE(app), slot_id, update_state_copy, &update);
}

static void status_set_message(GameStatus* status, const char* message) {
    status->message = kek_string_from_cstr(message);
}

static void app_set_input_state(WarehouseApp* app, const char* data, size_t len) {
    kek_standard_text_bridge_append(&app->input_bridge, data, len);
}

static void app_write_raw(WarehouseApp* app, const char* data, size_t len) {
    size_t written = kek_standard_output_write(app->stdout_stream, &app->output_bridge,
                                               data, len);
    if (written != len) {
        fprintf(stderr, "stdout stream buffer full, dropped %zu bytes\n", len - written);
    }
}

static void app_write(WarehouseApp* app, const char* text) {
    if (text) {
        app_write_raw(app, text, strlen(text));
    }
}

static void warehouse_reset(WarehouseApp* app) {
    size_t slots[] = {app->binding.slots.worker, app->binding.slots.warehouse_map,
                      app->binding.slots.package_a, app->binding.slots.package_b,
                      app->binding.slots.delivery_zone_a, app->binding.slots.delivery_zone_b,
                      app->binding.slots.game_status};
    const KekStateDescriptor* descriptors[sizeof(slots) / sizeof(slots[0])];
    KekStateStoreUpdateItem updates[sizeof(slots) / sizeof(slots[0])];
    for (size_t i = 0; i < sizeof(slots) / sizeof(slots[0]); i++) {
        descriptors[i] = kek_state_store_descriptor(APP_STORE(app), slots[i]);
        updates[i] = (KekStateStoreUpdateItem){slots[i], update_reset_state,
                                               (void*)descriptors[i]};
    }
    kek_state_store_update_many(APP_STORE(app), updates,
                                sizeof(updates) / sizeof(updates[0]));
}

static int warehouse_apply_package_rules(WarehouseApp* app, Worker* worker,
                                         Package packages[WAREHOUSE_PACKAGE_COUNT],
                                         GameStatus* status,
                                         int package_changed[WAREHOUSE_PACKAGE_COUNT]) {
    int changed = 0;

    for (size_t i = 0; i < WAREHOUSE_PACKAGE_COUNT; i++) {
        if (packages[i].status == PackageStatus_Waiting && !worker->carrying &&
            worker->x == packages[i].package_x && worker->y == packages[i].package_y) {
            packages[i].status = PackageStatus_Carried;
            worker->carrying = true;
            worker->score += 10;
            package_changed[i] = 1;
            changed = 1;
            status_set_message(status, "Package picked. Deliver it to the matching dock.");
            break;
        }
    }

    for (size_t i = 0; i < WAREHOUSE_PACKAGE_COUNT; i++) {
        const DeliveryZone* zone = app_delivery_zone_const(app, i);
        if (zone && packages[i].status == PackageStatus_Carried && worker->carrying &&
            worker->x == zone->zone_x && worker->y == zone->zone_y) {
            packages[i].status = PackageStatus_Delivered;
            worker->carrying = false;
            worker->score += 90;
            package_changed[i] = 1;
            changed = 1;
            status_set_message(status, "Package dropped at the matching dock.");
            break;
        }
    }
    return changed;
}

static char warehouse_cell_at(const WarehouseApp* app, int x, int y) {
    const Worker* worker = app_worker_const(app);

    if (worker->x == x && worker->y == y) {
        return worker->carrying ? 'W' : '@';
    }

    for (size_t i = 0; i < WAREHOUSE_PACKAGE_COUNT; i++) {
        const Package* package = app_package_const(app, i);
        if (package && package->status == PackageStatus_Waiting && !worker->carrying &&
            package->package_x == x && package->package_y == y) {
            return (char)('a' + (int)i);
        }
    }

    for (size_t i = 0; i < WAREHOUSE_PACKAGE_COUNT; i++) {
        const DeliveryZone* zone = app_delivery_zone_const(app, i);
        if (zone && zone->zone_x == x && zone->zone_y == y) {
            return (char)('A' + (int)i);
        }
    }
    return '.';
}

static const char* direction_name(Direction direction) {
    switch (direction) {
        case Direction_North:
            return "N";
        case Direction_South:
            return "S";
        case Direction_West:
            return "W";
        case Direction_East:
            return "E";
        case Direction_None:
        default:
            return "-";
    }
}

static void warehouse_render(WarehouseApp* app) {
    const Worker* worker = app_worker_const(app);
    const PlayerCommand* command = app_player_command_const(app);
    const WarehouseMap* map = app_warehouse_map_const(app);
    const GameStatus* status = app_game_status_const(app);
    KekString message = status->message;
    char screen[1000];
    size_t used = 0;

    int count = snprintf(screen + used, sizeof(screen) - used,
                         "\033[2J\033[H"
                         "Warehouse Picker - Kek state example\n"
                         "====================================\n"
                          "Goal: deliver packages 'a' and 'b' to docks 'A' and 'B'.\n"
                          "Controls: W/A/S/D or arrow keys move, R restarts, Q quits.\n\n"
                          "Energy: %d | Score: %d | Turn: %d | Carrying: %s | Facing: %s\n"
                          "Recent directions: %s %s %s %s\n\n",
                          worker->energy, worker->score, status->turn,
                          worker->carrying ? "yes" : "no", direction_name(worker->facing),
                          direction_name(command->recent_directions[0]),
                          direction_name(command->recent_directions[1]),
                          direction_name(command->recent_directions[2]),
                          direction_name(command->recent_directions[3]));
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
                     "\nLegend: @ worker, W carrying, a/b packages, A/B docks, # wall\n"
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

static Direction direction_from_delta(int dx, int dy) {
    if (dy < 0) {
        return Direction_North;
    }
    if (dy > 0) {
        return Direction_South;
    }
    if (dx < 0) {
        return Direction_West;
    }
    if (dx > 0) {
        return Direction_East;
    }
    return Direction_None;
}

static void warehouse_publish_command(WarehouseApp* app, int dx, int dy, int reset_requested,
                                      int quit_requested) {
    CommandUpdate update = {dx, dy, reset_requested, quit_requested,
                            direction_from_delta(dx, dy)};
    if (kek_state_store_update(APP_STORE(app), app->binding.slots.player_command,
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
        app_commit_state_copy(app, app->binding.slots.game_status, &status, sizeof(status));
        return;
    }

    Worker worker = *app_worker_const(app);
    Package packages[WAREHOUSE_PACKAGE_COUNT];
    for (size_t i = 0; i < WAREHOUSE_PACKAGE_COUNT; i++) {
        packages[i] = *app_package_const(app, i);
    }
    const WarehouseMap* map = app_warehouse_map_const(app);
    GameStatus status = *app_game_status_const(app);

    if (status.game_over) {
        status_set_message(&status, "The shift is over. Press R to restart or Q to quit.");
        app_commit_state_copy(app, app->binding.slots.game_status, &status, sizeof(status));
        return;
    }

    int next_x = worker.x + command->dx;
    int next_y = worker.y + command->dy;
    if (next_x < 1 || next_x > map->width - 2 || next_y < 1 || next_y > map->height - 2) {
        status_set_message(&status, "A shelf blocks that path.");
        app_commit_state_copy(app, app->binding.slots.game_status, &status, sizeof(status));
        return;
    }

    worker.x = next_x;
    worker.y = next_y;
    worker.energy--;
    worker.facing = command->recent_directions[3];
    status.turn++;
    status_set_message(&status, "You hurry through the warehouse aisles.");

    int package_changed[WAREHOUSE_PACKAGE_COUNT] = {0};
    warehouse_apply_package_rules(app, &worker, packages, &status, package_changed);

    if (worker.energy == 0 && !status.game_over) {
        status.game_over = true;
        status.won = false;
        status_set_message(&status, "Energy depleted before delivery.");
    }

    if (!Worker_check(&worker) || !GameStatus_check(&status)) {
        status_set_message(&status, "The requested move would put the worker in an invalid state.");
        app_commit_state_copy(app, app->binding.slots.game_status, &status, sizeof(status));
        return;
    }
    for (size_t i = 0; i < WAREHOUSE_PACKAGE_COUNT; i++) {
        if (!Package_check(&packages[i])) {
            return;
        }
    }

    StateCopyUpdate worker_update = {&worker, sizeof(worker)};
    StateCopyUpdate package_updates[WAREHOUSE_PACKAGE_COUNT];
    StateCopyUpdate status_update = {&status, sizeof(status)};
    KekStateStoreUpdateItem updates[WAREHOUSE_PACKAGE_COUNT + 2];
    size_t update_count = 0;
    updates[update_count++] = (KekStateStoreUpdateItem){app->binding.slots.worker,
                                                        update_state_copy,
                                                        &worker_update};
    for (size_t i = 0; i < WAREHOUSE_PACKAGE_COUNT; i++) {
        if (package_changed[i]) {
            package_updates[i] = (StateCopyUpdate){&packages[i], sizeof(packages[i])};
            updates[update_count++] = (KekStateStoreUpdateItem){app_package_slot(app, i),
                                                                update_state_copy,
                                                                &package_updates[i]};
        }
    }
    updates[update_count++] = (KekStateStoreUpdateItem){app->binding.slots.game_status,
                                                        update_state_copy,
                                                        &status_update};
    kek_state_store_update_many(APP_STORE(app), updates, update_count);
}

void UpdatePackageAfterWorkerChanged(KekHookContext* context) {
    WarehouseApp* app = (WarehouseApp*)context->app_context;
    if (!app || !context->event || context->event->state_slot_id != app->binding.slots.worker) {
        return;
    }
    const Worker* event_worker = (const Worker*)kek_hook_event_state(context, NULL);
    if (!event_worker) {
        return;
    }
    const Worker* current_worker = app_worker_const(app);
    if (!current_worker || current_worker->carrying == event_worker->carrying) {
        return;
    }
    Package packages[WAREHOUSE_PACKAGE_COUNT];
    for (size_t i = 0; i < WAREHOUSE_PACKAGE_COUNT; i++) {
        packages[i] = *app_package_const(app, i);
    }
    GameStatus status = *app_game_status_const(app);

    if (status.game_over) {
        return;
    }

    int package_changed[WAREHOUSE_PACKAGE_COUNT] = {0};
    if (event_worker->carrying) {
        status_set_message(&status, "Package picked. Deliver it to the matching dock.");
    } else {
        status_set_message(&status, "Package dropped at the matching dock.");
    }

    if (!GameStatus_check(&status)) {
        return;
    }
    for (size_t i = 0; i < WAREHOUSE_PACKAGE_COUNT; i++) {
        if (!Package_check(&packages[i])) {
            return;
        }
    }

    StateCopyUpdate package_updates[WAREHOUSE_PACKAGE_COUNT];
    StateCopyUpdate status_update = {&status, sizeof(status)};
    KekStateStoreUpdateItem updates[WAREHOUSE_PACKAGE_COUNT + 1];
    size_t update_count = 0;
    for (size_t i = 0; i < WAREHOUSE_PACKAGE_COUNT; i++) {
        if (package_changed[i]) {
            package_updates[i] = (StateCopyUpdate){&packages[i], sizeof(packages[i])};
            updates[update_count++] = (KekStateStoreUpdateItem){app_package_slot(app, i),
                                                                update_state_copy,
                                                                &package_updates[i]};
        }
    }
    updates[update_count++] = (KekStateStoreUpdateItem){app->binding.slots.game_status,
                                                        update_state_copy,
                                                        &status_update};
    kek_state_store_update_many(APP_STORE(app), updates, update_count);
}

void UpdateStatusAfterPackageChanged(KekHookContext* context) {
    WarehouseApp* app = (WarehouseApp*)context->app_context;
    if (!app || !context->event) {
        return;
    }
    int is_package_slot = 0;
    for (size_t i = 0; i < WAREHOUSE_PACKAGE_COUNT; i++) {
        is_package_slot = is_package_slot || context->event->state_slot_id == app_package_slot(app, i);
    }
    if (!is_package_slot) {
        return;
    }

    const Worker* worker = app_worker_const(app);
    GameStatus status = *app_game_status_const(app);
    int delivered_count = 0;
    for (size_t i = 0; i < WAREHOUSE_PACKAGE_COUNT; i++) {
        const Package* package = app_package_const(app, i);
        if (package && package->status == PackageStatus_Delivered) {
            delivered_count++;
        }
    }

    if (delivered_count == WAREHOUSE_PACKAGE_COUNT && !status.game_over) {
        status.game_over = true;
        status.won = true;
        status_set_message(&status, "All packages delivered on time.");
        app_commit_state_copy(app, app->binding.slots.game_status, &status, sizeof(status));
    } else if (worker->energy == 0 && !status.game_over) {
        status.game_over = true;
        status.won = false;
        status_set_message(&status, "Energy depleted before delivery.");
        app_commit_state_copy(app, app->binding.slots.game_status, &status, sizeof(status));
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

static void stream_eof_handler(const KekEvent* event, void* context) {
    WarehouseApp* app = (WarehouseApp*)context;
    if (event->source == app->stdin_stream) {
        kek_runtime_request_quit(app->runtime);
    }
}

static int app_init(WarehouseApp* app, KekRuntime* runtime, KekStream* stdin_stream,
                    KekStream* stdout_stream) {
    memset(app, 0, sizeof(*app));
    app->runtime = runtime;
    app->stdin_stream = stdin_stream;
    app->stdout_stream = stdout_stream;

    if (!warehouse_runtime_binding_init(&app->binding, runtime, app)) {
        return -1;
    }
    kek_standard_text_bridge_init(&app->input_bridge, APP_STORE(app),
                                  app->binding.slots.standard_input,
                                  app->binding.standard_input_input_buffer,
                                  sizeof(app->binding.standard_input_input_buffer),
                                  warehouse_standard_input_set_input);
    kek_standard_text_bridge_init(&app->output_bridge, APP_STORE(app),
                                  app->binding.slots.standard_output,
                                  app->binding.standard_output_output_buffer,
                                  sizeof(app->binding.standard_output_output_buffer),
                                  warehouse_standard_output_set_output);
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
    kek_event_subscribe(kek_runtime_events(&runtime), KEK_EVENT_STREAM_EOF,
                        stream_eof_handler, &app);
    kek_event_subscribe(kek_runtime_events(&runtime), KEK_EVENT_STREAM_ERROR,
                        stream_error_handler, &app);
    warehouse_render(&app);

    int result = kek_runtime_run(&runtime);
    kek_runtime_disable_raw_mode(&runtime, STDIN_FILENO);
    warehouse_runtime_binding_destroy(&app.binding);
    kek_runtime_destroy(&runtime);
    return result == 0 ? 0 : 1;
}
