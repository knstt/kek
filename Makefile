PYTHON ?= python3
CC ?= cc
CFLAGS ?= -std=c11 -Wall -Wextra -Werror
CPPFLAGS ?= -I. -Igenerated -Iruntime

SCHEMA := example/game.json
EXAMPLE_MAIN := example/main.c
GENERATED_DIR := generated
GENERATED_NAME := game
GENERATED_C := $(GENERATED_DIR)/$(GENERATED_NAME).c
GENERATED_H := $(GENERATED_DIR)/$(GENERATED_NAME).h
GENERATED_GRAPH := $(GENERATED_DIR)/$(GENERATED_NAME).graph.md

.PHONY: all generate check runtime editor clean

all: check runtime

generate: $(GENERATED_C) $(GENERATED_H) $(GENERATED_GRAPH)

$(GENERATED_C) $(GENERATED_H) $(GENERATED_GRAPH): $(SCHEMA) tools/generate_states.py
	$(PYTHON) tools/generate_states.py $(SCHEMA) --out-dir $(GENERATED_DIR) --name $(GENERATED_NAME)

check: generate
	$(CC) $(CFLAGS) -c $(GENERATED_C) -o $(GENERATED_DIR)/$(GENERATED_NAME).o

runtime: generate
	$(CC) $(CPPFLAGS) $(CFLAGS) $(EXAMPLE_MAIN) runtime/event.c runtime/hook.c runtime/stream.c runtime/runtime.c runtime/state_storage.c $(GENERATED_C) -o main

editor:
	$(PYTHON) tools/kek_editor.py example

clean:
	rm -f $(GENERATED_DIR)/$(GENERATED_NAME).o main keyboard_log.txt
