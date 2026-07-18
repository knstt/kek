CC ?= cc
AR ?= ar
CFLAGS ?= -std=c11 -Wall -Wextra -Werror
CPPFLAGS ?= -I. -Iruntime
UNAME_S := $(shell uname -s)

BUILD_DIR := build
LIB_DIR := lib
RUNTIME_LIB := $(LIB_DIR)/libkek_runtime.a
SMOKE_BIN := $(BUILD_DIR)/runtime_smoke
GAME_DIR := examples/game
GAME_GENERATED_DIR := $(GAME_DIR)/generated
GAME_STATE_SCHEMA := $(GAME_DIR)/game.schema.json
GAME_STATE_SRCS := $(GAME_GENERATED_DIR)/game_state.c $(GAME_GENERATED_DIR)/game_state.h
GAME_INC_SRCS := $(GAME_DIR)/game_logic.inc.c $(GAME_DIR)/game_hooks.inc.c $(GAME_DIR)/game_render.inc.c
GAME_BIN := $(BUILD_DIR)/game
RAYLIB_VERSION := 5.5

ifeq ($(UNAME_S),Darwin)
RAYLIB_PLATFORM := macos
GAME_LDLIBS := -lm -framework CoreVideo -framework IOKit -framework Cocoa -framework GLUT -framework OpenGL
else ifeq ($(UNAME_S),Linux)
RAYLIB_PLATFORM := linux_amd64
GAME_LDLIBS := -lm -ldl -lpthread -lrt -lX11 -lGL -lxcb -lXau -lXdmcp
else
$(error unsupported OS "$(UNAME_S)" for game builds)
endif

RAYLIB_DIR := $(GAME_DIR)/raylib-$(RAYLIB_VERSION)_$(RAYLIB_PLATFORM)
RAYLIB_ARCHIVE := $(GAME_DIR)/raylib-$(RAYLIB_VERSION)_$(RAYLIB_PLATFORM).tar.gz
RAYLIB_LIB := $(RAYLIB_DIR)/lib/libraylib.a
GAME_CPPFLAGS := $(CPPFLAGS) -I$(RAYLIB_DIR)/include
RUNTIME_SRCS := \
	runtime/event.c \
	runtime/hook.c \
	runtime/stream.c \
	runtime/standard_io.c \
	runtime/timer.c \
	runtime/runtime.c \
	runtime/state_storage.c \
	runtime/state_store.c
RUNTIME_OBJS := $(RUNTIME_SRCS:runtime/%.c=$(BUILD_DIR)/runtime/%.o)

.PHONY: all runtime smoke game run-game game-generate examples examples-clean clean

all: runtime

runtime: $(RUNTIME_LIB)

smoke: $(SMOKE_BIN)
	$(SMOKE_BIN)

game-generate: $(GAME_STATE_SRCS)

game: $(GAME_BIN)

run-game: $(GAME_BIN)
	$(GAME_BIN)

$(RUNTIME_LIB): $(RUNTIME_OBJS) | $(LIB_DIR)
	$(AR) rcs $@ $^

$(BUILD_DIR)/runtime/%.o: runtime/%.c | $(BUILD_DIR)/runtime
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(SMOKE_BIN): examples/runtime_smoke/main.c $(RUNTIME_LIB) | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) $< $(RUNTIME_LIB) -o $@

$(GAME_STATE_SRCS): $(GAME_STATE_SCHEMA) tools/generate_states.py
	python3 tools/generate_states.py $(GAME_STATE_SCHEMA) --out-dir $(GAME_GENERATED_DIR) --name game_state

$(RAYLIB_LIB): $(RAYLIB_ARCHIVE)
	tar -xzf $(RAYLIB_ARCHIVE) -C $(GAME_DIR)

$(GAME_BIN): $(GAME_DIR)/main.c $(GAME_INC_SRCS) $(GAME_GENERATED_DIR)/game_state.c $(GAME_GENERATED_DIR)/game_state.h $(RUNTIME_LIB) $(RAYLIB_LIB) | $(BUILD_DIR)
	$(CC) $(GAME_CPPFLAGS) $(CFLAGS) $(GAME_DIR)/main.c $(GAME_GENERATED_DIR)/game_state.c $(RUNTIME_LIB) $(RAYLIB_LIB) $(GAME_LDLIBS) -o $@

$(BUILD_DIR)/runtime $(LIB_DIR):
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR) $(LIB_DIR) main keyboard_log.txt $(GAME_DIR)/raylib-$(RAYLIB_VERSION)_linux_amd64 $(GAME_DIR)/raylib-$(RAYLIB_VERSION)_macos
