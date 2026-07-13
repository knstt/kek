#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "generated/game.h"
#include "runtime/runtime.h"
#include "runtime/stream.h"

#define GAME_TEXT_CAPACITY 1001

typedef struct GameApp {
    KekRuntime* runtime;
    KekStream* stdin_stream;
    KekStream* stdout_stream;
    KekStream* log_stream;
    GameState state;
    char input_buffer[GAME_TEXT_CAPACITY];
    char output_buffer[GAME_TEXT_CAPACITY];
    size_t input_len;
    size_t output_len;
} GameApp;

static void app_set_input_state(GameApp* app, const char* data, size_t len) {
    size_t available = sizeof(app->input_buffer) - app->input_len - 1;
    size_t to_copy = len < available ? len : available;
    if (to_copy > 0) {
        memcpy(app->input_buffer + app->input_len, data, to_copy);
        app->input_len += to_copy;
        app->input_buffer[app->input_len] = '\0';
    }

    app->state.standard_input.input.data = app->input_buffer;
    app->state.standard_input.input.len = app->input_len;
    StandardInput_verify(&app->state.standard_input);
    kek_runtime_publish_state_changed(app->runtime, &app->state.standard_input);
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

    app->state.standard_output.output.data = app->output_buffer;
    app->state.standard_output.output.len = app->output_len;
    StandardOutput_verify(&app->state.standard_output);
    kek_runtime_publish_state_changed(app->runtime, &app->state.standard_output);
}

static void app_write(GameApp* app, const char* text) {
    if (!text) {
        return;
    }
    size_t len = strlen(text);
    size_t written = kek_stream_write_raw(app->stdout_stream, text, len);
    if (written != len) {
        fprintf(stderr, "stdout stream buffer full, dropped %zu bytes\n", len - written);
    }
    app_track_output_state(app, text, written);
}

static void app_write_raw(GameApp* app, const char* data, size_t len) {
    size_t written = kek_stream_write_raw(app->stdout_stream, data, len);
    if (written != len) {
        fprintf(stderr, "stdout stream buffer full, dropped %zu bytes\n", len - written);
    }
    app_track_output_state(app, data, written);
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

    for (size_t i = 0; i < event->data_len; i++) {
        char ch = event->data[i];
        
        if (ch == 'q') {
            app_write(app, "\n\nQuitting...\n");
            kek_runtime_request_quit(app->runtime);
            break;
        }
        
        if (ch == '\n') {
            app_write(app, "[ENTER]\n");
        } else if (ch == '\x1b') {
            app_write(app, "[ESC]");
        } else if ((unsigned char)ch < 32) {
            char special[32];
            snprintf(special, sizeof(special), "[CTRL+%c]", ch + 64);
            app_write(app, special);
        } else {
            app_write_raw(app, &ch, 1);
        }
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
    app->state = GameState_default();
    GameState_verify(&app->state);
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
    app_init(&app, &runtime, kek_runtime_get_stream(&runtime, (size_t)stdin_id),
             kek_runtime_get_stream(&runtime, (size_t)stdout_id),
             kek_runtime_get_stream(&runtime, (size_t)log_id));

    kek_event_subscribe(kek_runtime_events(&runtime), KEK_EVENT_STREAM_DATA,
                        stream_data_handler, &app);
    kek_event_subscribe(kek_runtime_events(&runtime), KEK_EVENT_STREAM_ERROR,
                        stream_error_handler, &app);

    app_write(&app, "Raw keyboard input mode - Press 'q' to quit\n");
    app_write(&app, "Characters: ");

    int result = kek_runtime_run(&runtime);
    kek_runtime_disable_raw_mode(&runtime, STDIN_FILENO);
    kek_runtime_destroy(&runtime);

    printf("Keyboard log saved to keyboard_log.txt\n");
    return result == 0 ? 0 : 1;
}
