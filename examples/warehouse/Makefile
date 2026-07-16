PYTHON ?= python3
CC ?= cc
CFLAGS ?= -std=c11 -Wall -Wextra -Werror

ROOT := ../..
CPPFLAGS ?= -I. -I$(ROOT) -I$(ROOT)/runtime

SCHEMA := warehouse.json
EXAMPLE_MAIN := main.c
GENERATED_DIR := generated
GENERATED_NAME := warehouse
GENERATED_C := $(GENERATED_DIR)/$(GENERATED_NAME).c
GENERATED_H := $(GENERATED_DIR)/$(GENERATED_NAME).h
GENERATED_GRAPH := $(GENERATED_DIR)/$(GENERATED_NAME).graph.md
RUNTIME_LIB := $(ROOT)/lib/libkek_runtime.a
TARGET := warehouse

.PHONY: all generate check run clean

all: check $(TARGET)

generate: $(GENERATED_C) $(GENERATED_H) $(GENERATED_GRAPH)

$(GENERATED_C) $(GENERATED_H) $(GENERATED_GRAPH): $(SCHEMA) $(ROOT)/tools/generate_states.py
	$(PYTHON) $(ROOT)/tools/generate_states.py $(SCHEMA) --out-dir $(GENERATED_DIR) --name $(GENERATED_NAME)

check: generate
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $(GENERATED_C) -o $(GENERATED_DIR)/$(GENERATED_NAME).o

$(RUNTIME_LIB):
	$(MAKE) -C $(ROOT) runtime

$(TARGET): generate $(EXAMPLE_MAIN) $(RUNTIME_LIB)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(EXAMPLE_MAIN) $(GENERATED_C) $(RUNTIME_LIB) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) $(GENERATED_DIR)/$(GENERATED_NAME).o $(GENERATED_C) $(GENERATED_H) $(GENERATED_GRAPH)
