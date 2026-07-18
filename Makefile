CC ?= cc
AR ?= ar
CFLAGS ?= -std=c11 -Wall -Wextra -Werror
CPPFLAGS ?= -I. -Iruntime
UNAME_S := $(shell uname -s)

BUILD_DIR := build
LIB_DIR := lib
RUNTIME_LIB := $(LIB_DIR)/libkek_runtime.a
RUNTIME_DYNAMIC_LIB := $(LIB_DIR)/libkek_runtime_dynamic.a
SMOKE_BIN := $(BUILD_DIR)/runtime_smoke
DYNAMIC_HOOK_SMOKE_DIR := examples/runtime_hook_dynamic
DYNAMIC_HOOK_SMOKE_BIN := $(BUILD_DIR)/runtime_hook_dynamic_smoke
GAME_DIR := examples/game
GAME_GENERATED_DIR := $(GAME_DIR)/generated
GAME_HOOK_DIR := $(GAME_DIR)/hooks
GAME_STATE_SCHEMA := $(GAME_DIR)/game.schema.json
GAME_STATE_SRCS := $(GAME_GENERATED_DIR)/game_state.c $(GAME_GENERATED_DIR)/game_state.h
GAME_INC_SRCS := $(GAME_DIR)/game_logic.inc.c $(GAME_DIR)/game_render.inc.c
GAME_SHARED_SRCS := $(GAME_DIR)/game_update_helpers.c
GAME_HOOK_SRCS := \
	$(GAME_HOOK_DIR)/game_hook_support.c \
	$(GAME_HOOK_DIR)/on_frame_clock.c \
	$(GAME_HOOK_DIR)/move_grunt_enemy.c \
	$(GAME_HOOK_DIR)/move_runner_enemy.c \
	$(GAME_HOOK_DIR)/move_tank_enemy.c \
	$(GAME_HOOK_DIR)/move_boss_enemy.c \
	$(GAME_HOOK_DIR)/on_player_health_changed.c \
	$(GAME_HOOK_DIR)/on_wave_changed.c \
	$(GAME_HOOK_DIR)/on_score_changed.c
GAME_BIN := $(BUILD_DIR)/game
GAME_DEBUG_BIN := $(BUILD_DIR)/game-debug-hooks
RAYLIB_VERSION := 5.5

ifeq ($(UNAME_S),Darwin)
RAYLIB_PLATFORM := macos
GAME_LDLIBS := -lm -framework CoreVideo -framework IOKit -framework Cocoa -framework GLUT -framework OpenGL
RUNTIME_DYNAMIC_LDLIBS :=
SHARED_EXT := dylib
SHARED_LDFLAGS := -dynamiclib -undefined dynamic_lookup
EXPORT_DYNAMIC := -Wl,-export_dynamic
else ifeq ($(UNAME_S),Linux)
RAYLIB_PLATFORM := linux_amd64
GAME_LDLIBS := -lm -ldl -lpthread -lrt -lX11 -lGL -lxcb -lXau -lXdmcp
RUNTIME_DYNAMIC_LDLIBS := -ldl
SHARED_EXT := so
SHARED_LDFLAGS := -shared
EXPORT_DYNAMIC := -rdynamic
else
$(error unsupported OS "$(UNAME_S)" for game builds)
endif

RAYLIB_DIR := $(GAME_DIR)/raylib-$(RAYLIB_VERSION)_$(RAYLIB_PLATFORM)
RAYLIB_ARCHIVE := $(GAME_DIR)/raylib-$(RAYLIB_VERSION)_$(RAYLIB_PLATFORM).tar.gz
RAYLIB_LIB := $(RAYLIB_DIR)/lib/libraylib.a
GAME_CPPFLAGS := $(CPPFLAGS) -I$(GAME_GENERATED_DIR) -I$(RAYLIB_DIR)/include
RUNTIME_SRCS := \
	runtime/app.c \
	runtime/event.c \
	runtime/hook.c \
	runtime/stream.c \
	runtime/standard_io.c \
	runtime/timer.c \
	runtime/runtime.c \
	runtime/state_storage.c \
	runtime/state_store.c
RUNTIME_OBJS := $(RUNTIME_SRCS:runtime/%.c=$(BUILD_DIR)/runtime/%.o)
RUNTIME_DYNAMIC_OBJS := $(RUNTIME_SRCS:runtime/%.c=$(BUILD_DIR)/runtime_dynamic/%.o)
GAME_HOOK_LIB := $(BUILD_DIR)/game_hooks.$(SHARED_EXT)
GAME_DEBUG_HOOK_LIBRARY := $(abspath $(GAME_HOOK_LIB))
DYNAMIC_HOOK_SMOKE_V1_LIB := $(BUILD_DIR)/runtime_hook_v1.$(SHARED_EXT)
DYNAMIC_HOOK_SMOKE_V2_LIB := $(BUILD_DIR)/runtime_hook_v2.$(SHARED_EXT)
DYNAMIC_HOOK_SMOKE_MISSING_LIB := $(BUILD_DIR)/runtime_hook_missing.$(SHARED_EXT)

.PHONY: all runtime smoke dynamic-hook-smoke game game-debug-hooks run-game run-game-debug-hooks game-generate examples examples-clean clean

all: runtime

runtime: $(RUNTIME_LIB)

smoke: $(SMOKE_BIN)
	$(SMOKE_BIN)

dynamic-hook-smoke: $(DYNAMIC_HOOK_SMOKE_BIN) $(DYNAMIC_HOOK_SMOKE_V1_LIB) $(DYNAMIC_HOOK_SMOKE_MISSING_LIB) $(DYNAMIC_HOOK_SMOKE_V2_LIB)
	$(DYNAMIC_HOOK_SMOKE_BIN) $(abspath $(DYNAMIC_HOOK_SMOKE_V1_LIB)) $(abspath $(DYNAMIC_HOOK_SMOKE_MISSING_LIB)) $(abspath $(DYNAMIC_HOOK_SMOKE_V2_LIB))

game-generate: $(GAME_STATE_SRCS)

game: $(GAME_BIN)

game-debug-hooks: $(GAME_DEBUG_BIN) $(GAME_HOOK_LIB)

run-game: $(GAME_BIN)
	$(GAME_BIN)

run-game-debug-hooks: $(GAME_DEBUG_BIN) $(GAME_HOOK_LIB)
	$(GAME_DEBUG_BIN)

$(RUNTIME_LIB): $(RUNTIME_OBJS) | $(LIB_DIR)
	$(AR) rcs $@ $^

$(RUNTIME_DYNAMIC_LIB): $(RUNTIME_DYNAMIC_OBJS) | $(LIB_DIR)
	$(AR) rcs $@ $^

$(BUILD_DIR)/runtime/%.o: runtime/%.c | $(BUILD_DIR)/runtime
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/runtime_dynamic/%.o: runtime/%.c | $(BUILD_DIR)/runtime_dynamic
	$(CC) $(CPPFLAGS) -DKEK_HOOK_DYNAMIC $(CFLAGS) -c $< -o $@

$(SMOKE_BIN): examples/runtime_smoke/main.c $(RUNTIME_LIB) | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) $< $(RUNTIME_LIB) -o $@

$(DYNAMIC_HOOK_SMOKE_BIN): $(DYNAMIC_HOOK_SMOKE_DIR)/main.c $(DYNAMIC_HOOK_SMOKE_DIR)/dynamic_hook_smoke.h $(RUNTIME_DYNAMIC_LIB) | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) -DKEK_HOOK_DYNAMIC $(CFLAGS) $(EXPORT_DYNAMIC) $< $(RUNTIME_DYNAMIC_LIB) $(RUNTIME_DYNAMIC_LDLIBS) -o $@

$(DYNAMIC_HOOK_SMOKE_V1_LIB): $(DYNAMIC_HOOK_SMOKE_DIR)/hook_v1.c $(DYNAMIC_HOOK_SMOKE_DIR)/dynamic_hook_smoke.h | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) -DKEK_HOOK_DYNAMIC $(CFLAGS) -fPIC $(SHARED_LDFLAGS) $< -o $@

$(DYNAMIC_HOOK_SMOKE_V2_LIB): $(DYNAMIC_HOOK_SMOKE_DIR)/hook_v2.c $(DYNAMIC_HOOK_SMOKE_DIR)/dynamic_hook_smoke.h | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) -DKEK_HOOK_DYNAMIC $(CFLAGS) -fPIC $(SHARED_LDFLAGS) $< -o $@

$(DYNAMIC_HOOK_SMOKE_MISSING_LIB): $(DYNAMIC_HOOK_SMOKE_DIR)/hook_missing.c $(DYNAMIC_HOOK_SMOKE_DIR)/dynamic_hook_smoke.h | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) -DKEK_HOOK_DYNAMIC $(CFLAGS) -fPIC $(SHARED_LDFLAGS) $< -o $@

$(GAME_STATE_SRCS): $(GAME_STATE_SCHEMA) tools/generate_states.py
	python3 tools/generate_states.py $(GAME_STATE_SCHEMA) --out-dir $(GAME_GENERATED_DIR) --name game_state --hooks-dir $(GAME_HOOK_DIR)

$(RAYLIB_LIB): $(RAYLIB_ARCHIVE)
	tar -xzf $(RAYLIB_ARCHIVE) -C $(GAME_DIR)

$(GAME_BIN): $(GAME_DIR)/main.c $(GAME_DIR)/game_app.h $(GAME_INC_SRCS) $(GAME_SHARED_SRCS) $(GAME_HOOK_SRCS) $(GAME_GENERATED_DIR)/game_state.c $(GAME_GENERATED_DIR)/game_state.h $(RUNTIME_LIB) $(RAYLIB_LIB) | $(BUILD_DIR)
	$(CC) $(GAME_CPPFLAGS) $(CFLAGS) $(GAME_DIR)/main.c $(GAME_SHARED_SRCS) $(GAME_HOOK_SRCS) $(GAME_GENERATED_DIR)/game_state.c $(RUNTIME_LIB) $(RAYLIB_LIB) $(GAME_LDLIBS) -o $@

$(GAME_HOOK_LIB): $(GAME_HOOK_SRCS) $(GAME_SHARED_SRCS) $(GAME_GENERATED_DIR)/game_state.c $(GAME_GENERATED_DIR)/game_state.h | $(BUILD_DIR)
	$(CC) $(GAME_CPPFLAGS) $(CFLAGS) -fPIC $(SHARED_LDFLAGS) $(GAME_SHARED_SRCS) $(GAME_HOOK_SRCS) $(GAME_GENERATED_DIR)/game_state.c -o $@

$(GAME_DEBUG_BIN): $(GAME_DIR)/main.c $(GAME_DIR)/game_app.h $(GAME_INC_SRCS) $(GAME_SHARED_SRCS) $(GAME_GENERATED_DIR)/game_state.c $(GAME_GENERATED_DIR)/game_state.h $(RUNTIME_DYNAMIC_LIB) $(RAYLIB_LIB) $(GAME_HOOK_LIB) | $(BUILD_DIR)
	$(CC) $(GAME_CPPFLAGS) -DKEK_HOOK_DYNAMIC -DKEK_GAME_HOOK_LIBRARY=\"$(GAME_DEBUG_HOOK_LIBRARY)\" $(CFLAGS) $(EXPORT_DYNAMIC) $(GAME_DIR)/main.c $(GAME_SHARED_SRCS) $(GAME_GENERATED_DIR)/game_state.c $(RUNTIME_DYNAMIC_LIB) $(RAYLIB_LIB) $(GAME_LDLIBS) -o $@

$(BUILD_DIR)/runtime $(BUILD_DIR)/runtime_dynamic $(LIB_DIR):
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR) $(LIB_DIR) main keyboard_log.txt $(GAME_DIR)/raylib-$(RAYLIB_VERSION)_linux_amd64 $(GAME_DIR)/raylib-$(RAYLIB_VERSION)_macos
