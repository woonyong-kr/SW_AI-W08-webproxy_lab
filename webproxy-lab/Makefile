# 프록시 랩용 Makefile
#
# `handin` 규칙을 제외하면 이 파일은 자유롭게 수정해도 됩니다.
# 교수자나 조교는 여러분의 Makefile에서 `make`를 실행해 프록시를
# 소스에서 빌드합니다.

CC = gcc
CFLAGS = -g -Wall
LDFLAGS = -lpthread
OS = $(shell uname -s)
ARCH = $(shell uname -m)
PLATFORM = $(OS)-$(ARCH)
BUILD_DIR = .build/$(PLATFORM)
PROXY_BIN = $(BUILD_DIR)/proxy.bin
CSAPP_OBJ = $(BUILD_DIR)/csapp.o
PROXY_OBJ = $(BUILD_DIR)/proxy.o

.PHONY: all clean handin

all: proxy

$(BUILD_DIR):
	mkdir -p $@

$(CSAPP_OBJ): csapp.c csapp.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c csapp.c -o $@

$(PROXY_OBJ): proxy.c csapp.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c proxy.c -o $@

$(PROXY_BIN): $(PROXY_OBJ) $(CSAPP_OBJ)
	$(CC) $(CFLAGS) $(PROXY_OBJ) $(CSAPP_OBJ) -o $@ $(LDFLAGS)

proxy: $(PROXY_BIN)
	printf '%s\n' \
		'#!/bin/sh' \
		'set -eu' \
		'DIR=$$(CDPATH= cd -- "$$(dirname "$$0")" && pwd)' \
		'BIN="$$DIR/$(PROXY_BIN)"' \
		'make -C "$$DIR" "$(PROXY_BIN)"' \
		'exec "$$BIN" "$$@"' > $@
	chmod +x $@

# 제출할 수 있는 tar 파일을 ../proxylab-handin.tar에 생성합니다.
# 이 규칙은 수정하지 마세요.
handin:
	(make clean; cd ..; tar cvf $(USER)-proxylab-handin.tar proxylab-handout --exclude tiny --exclude nop-server.py --exclude proxy --exclude driver.sh --exclude port-for-user.pl --exclude free-port.sh --exclude ".*")

clean:
	rm -rf *~ *.o proxy .build core *.tar *.zip *.gzip *.bzip *.gz
