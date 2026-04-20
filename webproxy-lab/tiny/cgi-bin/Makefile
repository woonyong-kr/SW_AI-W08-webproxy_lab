CC = gcc
CFLAGS = -O2 -Wall -I ..
OS = $(shell uname -s)
ARCH = $(shell uname -m)
PLATFORM = $(OS)-$(ARCH)
BUILD_DIR = .build/$(PLATFORM)
ADDER_BIN = $(BUILD_DIR)/adder.bin

all: adder

$(BUILD_DIR):
	mkdir -p $@

$(ADDER_BIN): adder.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ adder.c

adder: $(ADDER_BIN)
	printf '%s\n' \
		'#!/bin/sh' \
		'set -eu' \
		'DIR=$$(CDPATH= cd -- "$$(dirname "$$0")" && pwd)' \
		'BIN="$$DIR/$(ADDER_BIN)"' \
		'make -C "$$DIR" "$(ADDER_BIN)"' \
		'exec "$$BIN" "$$@"' > $@
	chmod +x $@

clean:
	rm -rf .build adder *~
