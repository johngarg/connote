CC = gcc
CFLAGS = -Wall -Wextra -Wno-unused-but-set-parameter -Wno-unused-parameter -I./src
BIN_DIR = bin

all: connote test

connote: src/connote.c src/config.c src/utils.c
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $(BIN_DIR)/note $^

test: tests/test_*.c src/utils.c
	$(CC) $(CFLAGS) -o $(BIN_DIR)/$@ $^
	./bin/test

.PHONY: all clean
