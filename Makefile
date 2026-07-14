PYTHON ?= python3
CC ?= cc
CFLAGS ?= -std=c11 -Wall -Wextra -Werror
CPPFLAGS ?= -Igenerated -Iruntime

SCHEMA := game.kek
GENERATED_DIR := generated
GENERATED_NAME := game
GENERATED_C := $(GENERATED_DIR)/$(GENERATED_NAME).c
GENERATED_H := $(GENERATED_DIR)/$(GENERATED_NAME).h

.PHONY: all generate check runtime clean

all: check runtime

generate: $(GENERATED_C) $(GENERATED_H)

$(GENERATED_C) $(GENERATED_H): $(SCHEMA) tools/generate_states.py
	$(PYTHON) tools/generate_states.py $(SCHEMA) --out-dir $(GENERATED_DIR) --name $(GENERATED_NAME)

check: generate
	$(CC) $(CFLAGS) -c $(GENERATED_C) -o $(GENERATED_DIR)/$(GENERATED_NAME).o

runtime: generate
	$(CC) $(CPPFLAGS) $(CFLAGS) main.c runtime/event.c runtime/hook.c runtime/stream.c runtime/runtime.c runtime/state_storage.c $(GENERATED_C) -o main

clean:
	rm -f $(GENERATED_DIR)/$(GENERATED_NAME).o main keyboard_log.txt
