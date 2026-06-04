# Simple Makefile for a small C project.
# Configure the C source files explicitly via the SRCS variable.
PROJECT ?= kek
BIN_DIR ?= bin
SRCS ?=main.c ast.c tokenizer.c ast_json.c source.c codegen_c.c
CFLAGS ?= -O2 -Wall
CC := $(shell command -v gcc 2>/dev/null || command -v cc 2>/dev/null || true)
INSTALL_DIR ?= /usr/local/bin

.PHONY: help build c-build test fmt lint clean install noop-build

help:
	@printf "Usage:\n"
	@printf "  make build          Build the project (set SRCS manually)\n"
	@printf "  make test           Run tests (no test runner configured)\n"
	@printf "  make fmt            Format sources (no formatter configured)\n"
	@printf "  make lint           Run a linter if available\n"
	@printf "  make install        Install binary to $(INSTALL_DIR)\n"
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
	@echo "No test runner configured for this project type."

fmt:
	@echo "No formatter configured for this project type."

lint:
	@if command -v cppcheck >/dev/null 2>&1 && [ -n "$(SRCS)" ]; then \
		echo "Running cppcheck..."; \
		cppcheck --enable=all $(SRCS); \
	else \
		echo "No linter found or SRCS is empty."; \
	fi

install: build
	@mkdir -p $(INSTALL_DIR)
	@if [ ! -f "$(BIN_DIR)/$(PROJECT)" ]; then echo "Build artifact not found: $(BIN_DIR)/$(PROJECT)"; exit 1; fi
	@echo "Installing $(BIN_DIR)/$(PROJECT) -> $(INSTALL_DIR)/$(PROJECT)"
	@cp "$(BIN_DIR)/$(PROJECT)" "$(INSTALL_DIR)/$(PROJECT)"
	@echo "Installed."

clean:
	@echo "Cleaning build artifacts..."
	@rm -rf $(BIN_DIR)