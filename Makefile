CC ?= cc
AR ?= ar
CFLAGS ?= -std=c11 -Wall -Wextra -Werror
CPPFLAGS ?= -I. -Iruntime

BUILD_DIR := build
LIB_DIR := lib
RUNTIME_LIB := $(LIB_DIR)/libkek_runtime.a
RUNTIME_SRCS := \
	runtime/event.c \
	runtime/hook.c \
	runtime/stream.c \
	runtime/timer.c \
	runtime/runtime.c \
	runtime/state_storage.c
RUNTIME_OBJS := $(RUNTIME_SRCS:runtime/%.c=$(BUILD_DIR)/runtime/%.o)

.PHONY: all runtime clean

all: runtime

runtime: $(RUNTIME_LIB)

$(RUNTIME_LIB): $(RUNTIME_OBJS) | $(LIB_DIR)
	$(AR) rcs $@ $^

$(BUILD_DIR)/runtime/%.o: runtime/%.c | $(BUILD_DIR)/runtime
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/runtime $(LIB_DIR):
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR) $(LIB_DIR) main keyboard_log.txt
