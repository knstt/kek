CC ?= cc
AR ?= ar
CFLAGS ?= -std=c11 -Wall -Wextra -Werror
CPPFLAGS ?= -I. -Iruntime

BUILD_DIR := build
LIB_DIR := lib
RUNTIME_LIB := $(LIB_DIR)/libkek_runtime.a
SMOKE_BIN := $(BUILD_DIR)/runtime_smoke
RUNTIME_SRCS := \
	runtime/event.c \
	runtime/hook.c \
	runtime/stream.c \
	runtime/standard_io.c \
	runtime/timer.c \
	runtime/runtime.c \
	runtime/state_storage.c
RUNTIME_OBJS := $(RUNTIME_SRCS:runtime/%.c=$(BUILD_DIR)/runtime/%.o)

.PHONY: all runtime smoke examples examples-clean clean

all: runtime

runtime: $(RUNTIME_LIB)

smoke: $(SMOKE_BIN)
	$(SMOKE_BIN)

$(RUNTIME_LIB): $(RUNTIME_OBJS) | $(LIB_DIR)
	$(AR) rcs $@ $^

$(BUILD_DIR)/runtime/%.o: runtime/%.c | $(BUILD_DIR)/runtime
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(SMOKE_BIN): examples/runtime_smoke/main.c $(RUNTIME_LIB) | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) $< $(RUNTIME_LIB) -o $@

$(BUILD_DIR)/runtime $(LIB_DIR):
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR) $(LIB_DIR) main keyboard_log.txt
