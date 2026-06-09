# Simple Makefile for a small C project.
# Configure the C source files explicitly via the SRCS variable.
PROJECT ?= kek
BIN_DIR ?= bin
SRCS ?=main.c ast.c parser.c sema.c tokenizer.c ast_json.c source.c codegen_c.c
CFLAGS ?= -std=c11 -O2 -Wall -Wextra -pedantic
CC := $(shell command -v gcc 2>/dev/null || command -v cc 2>/dev/null || true)
INSTALL_DIR ?= /usr/local/bin
SMOKE_BIN ?= out/tmp
SMOKE_EXPECTED_EXIT ?= 91
PYTHON ?= python3

.PHONY: help build c-build test fmt lint clean install noop-build

help:
	@printf "Usage:\n"
	@printf "  make build          Build the project (set SRCS manually)\n"
	@printf "  make test           Build and run the tmp.kek smoke test\n"
	@printf "  make clean          Remove build artifacts\n"
	@printf "\nSet the SRCS variable in this Makefile or pass it on the make command line, e.g.\n"
	@printf "  make SRCS=\"main.c source.c\" build\n"

# Default: build using SRCS if provided
build: $(if $(SRCS),c-build,noop-build)

noop-build:
	@echo "No source files defined. Set SRCS in the Makefile or pass SRCS='file1.c file2.c' to make."

# C build (combine listed source files into single binary)
c-build:
	@if [ -z "$(CC)" ]; then echo "C compiler not found in PATH"; exit 1; fi
	@if [ -z "$(SRCS)" ]; then echo "No source files defined (SRCS is empty)"; exit 1; fi
	@mkdir -p $(BIN_DIR)
	@echo "Building C project -> $(BIN_DIR)/$(PROJECT)"
	@$(CC) $(CFLAGS) -o $(BIN_DIR)/$(PROJECT) $(SRCS)

test:
	@$(MAKE) build
	@echo "Compiling tmp.kek -> out/out.c"
	@$(BIN_DIR)/$(PROJECT)
	@cp out/out.c out/out.pretty.c
	@$(PYTHON) tools/normalize_c.py < out/out.pretty.c > out/out.pretty.norm.c
	@echo "Building generated C -> $(SMOKE_BIN)"
	@$(CC) $(CFLAGS) -o $(SMOKE_BIN) out/out.c
	@echo "Running smoke binary"
	@set +e; \
	$(SMOKE_BIN); \
	status=$$?; \
	set -e; \
	if [ $$status -ne $(SMOKE_EXPECTED_EXIT) ]; then \
		echo "Smoke test failed: expected exit $(SMOKE_EXPECTED_EXIT), got $$status"; \
		exit 1; \
	fi; \
	echo "Smoke test passed: exit $$status"
	@echo "Checking whitespace-agnostic smoke output"
	@cp tmp.kek out/tmp.original.kek
	@$(PYTHON) tools/minify_kek.py < tmp.kek > out/tmp.min.kek
	@trap 'cp out/tmp.original.kek tmp.kek' EXIT; \
	cp out/tmp.min.kek tmp.kek; \
	$(BIN_DIR)/$(PROJECT); \
	$(PYTHON) tools/normalize_c.py < out/out.c > out/out.min.norm.c; \
	diff -u out/out.pretty.norm.c out/out.min.norm.c >/dev/null
	@echo "Whitespace smoke test passed"

clean:
	@echo "Cleaning build artifacts..."
	@rm -rf $(BIN_DIR) out
