CC ?= cc
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -Werror -g
CPPFLAGS ?= -Iinclude

BIN := build/keklex
TEST_BIN := build/test_lexer

.PHONY: all test clean examples

all: $(BIN)

$(BIN): src/main.c src/lexer.c include/kek/lexer.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) src/main.c src/lexer.c -o $@

$(TEST_BIN): tests/test_lexer.c src/lexer.c include/kek/lexer.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_lexer.c src/lexer.c -o $@

build:
	mkdir -p build

test: $(TEST_BIN)
	$(TEST_BIN)

examples: $(BIN)
	$(BIN) example/main.kek >/dev/null
	$(BIN) example/docs.kek >/dev/null
	$(BIN) example/future.kek >/dev/null
	$(BIN) example/MyPackage/something.kek >/dev/null

clean:
	rm -rf build
