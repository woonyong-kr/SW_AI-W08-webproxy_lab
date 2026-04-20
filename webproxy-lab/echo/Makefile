CC ?= gcc
CFLAGS ?= -g -Wall -Wextra -std=c11

OS := $(shell uname -s)
ARCH := $(shell uname -m)
PLATFORM := $(OS)-$(ARCH)
BUILD_DIR := .build/$(PLATFORM)
SERVER_BIN := $(BUILD_DIR)/echo-server.bin
CLIENT_BIN := $(BUILD_DIR)/echo-client.bin
DEFAULT_PORT := 9000
DEFAULT_HOST := 127.0.0.1

.PHONY: all run-server run-client clean clean-all print-platform

all: $(SERVER_BIN) $(CLIENT_BIN)

print-platform:
	@echo $(PLATFORM)

$(BUILD_DIR):
	mkdir -p $@

$(SERVER_BIN): echo-server.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $<

$(CLIENT_BIN): echo-client.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $<

run-server: $(SERVER_BIN)
	./$(SERVER_BIN) $(or $(PORT),$(DEFAULT_PORT))

run-client: $(CLIENT_BIN)
	printf '%s\n' "$(or $(MESSAGE),hello echo)" | ./$(CLIENT_BIN) $(or $(HOST),$(DEFAULT_HOST)) $(or $(PORT),$(DEFAULT_PORT))

clean:
	rm -rf $(BUILD_DIR)

clean-all:
	rm -rf .build
