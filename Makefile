# Simple Makefile for a small C project.
# Configure the C source files explicitly via the SRCS variable.
PROJECT ?= kek
BIN_DIR ?= bin
CORE_SRCS ?= ast.c parser.c sema.c tokenizer.c ast_json.c source.c diagnostics.c compilation.c codegen_c.c
SRCS ?= main.c $(CORE_SRCS)
CFLAGS ?= -std=c11 -O2 -Wall -Wextra -pedantic
GENERATED_CFLAGS ?= $(CFLAGS) -Werror
CC := $(shell command -v gcc 2>/dev/null || command -v cc 2>/dev/null || true)
INSTALL_DIR ?= /usr/local/bin
SMOKE_BIN ?= out/tmp
SMOKE_EXPECTED_EXIT ?= 91
PYTHON ?= python3

.PHONY: help build c-build test api-test kekfmt kekfmt-test fmt lint clean install noop-build

help:
	@printf "Usage:\n"
	@printf "  make build          Build the compiler into $(BIN_DIR)/$(PROJECT)\n"
	@printf "  make test           Build and run the tmp.kek smoke test\n"
	@printf "  make kekfmt         Build the Kek formatter into out/kekfmt\n"
	@printf "  make fmt            Normalize generated smoke C output, if present\n"
	@printf "  make lint           Build with the configured warning flags\n"
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
	@$(CC) $(GENERATED_CFLAGS) -o $(SMOKE_BIN) out/out.c
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
	@$(MAKE) api-test
	@$(MAKE) kekfmt-test

api-test:
	@if [ -z "$(CC)" ]; then echo "C compiler not found in PATH"; exit 1; fi
	@mkdir -p out
	@$(MAKE) build
	@$(BIN_DIR)/$(PROJECT)
	@echo "Building API/tooling tests -> out/api_tests"
	@$(CC) $(CFLAGS) -o out/api_tests tests.c $(CORE_SRCS)
	@out/api_tests

kekfmt:
	@if [ -z "$(CC)" ]; then echo "C compiler not found in PATH"; exit 1; fi
	@$(MAKE) build
	@mkdir -p out
	@set -e; \
	tmp_backup=$$(mktemp out/tmp.kek.XXXXXX); \
	cp tmp.kek $$tmp_backup; \
	trap 'cp $$tmp_backup tmp.kek; rm -f $$tmp_backup' EXIT; \
	cp fmt.kek tmp.kek; \
	$(BIN_DIR)/$(PROJECT); \
	$(CC) $(GENERATED_CFLAGS) -o out/kekfmt out/out.c
	@echo "Wrote out/kekfmt"

kekfmt-test: kekfmt
	@cp fmt.kek out/fmt.formatted.kek
	@out/kekfmt out/fmt.formatted.kek
	@set -e; \
	tmp_backup=$$(mktemp out/tmp.kek.XXXXXX); \
	cp tmp.kek $$tmp_backup; \
	trap 'cp $$tmp_backup tmp.kek; rm -f $$tmp_backup' EXIT; \
	cp out/fmt.formatted.kek tmp.kek; \
	$(BIN_DIR)/$(PROJECT); \
	$(CC) $(GENERATED_CFLAGS) -o out/kekfmt.formatted out/out.c
	@echo "Kek formatter self-test passed"

fmt:
	@if [ -f out/out.c ]; then \
		$(PYTHON) tools/normalize_c.py < out/out.c > out/out.norm.c; \
		echo "Wrote out/out.norm.c"; \
	else \
		echo "Nothing to format: out/out.c does not exist"; \
	fi

lint: build

clean:
	@echo "Cleaning build artifacts..."
	@rm -rf $(BIN_DIR) out
