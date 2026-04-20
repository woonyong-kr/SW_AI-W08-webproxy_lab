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
DEFAULT_CONTAINER := $(shell command -v docker >/dev/null 2>&1 && docker ps --format '{{.Names}}' | head -n 1)
CONTAINER ?= $(DEFAULT_CONTAINER)
CONTAINER_WORKDIR ?= /workspaces/SW_AI-W08-webproxy_lab/webproxy-lab/echo
CLIENT_COUNT ?= 1
SECOND_GOAL := $(word 2,$(MAKECMDGOALS))
ifneq ($(filter open-client open-both,$(MAKECMDGOALS)),)
ifneq ($(SECOND_GOAL),)
CLIENT_COUNT := $(SECOND_GOAL)
$(eval .PHONY: $(SECOND_GOAL))
$(eval $(SECOND_GOAL): ; @:)
endif
endif

.PHONY: all run-server run-client run-client-interactive docker-stop-server docker-status-server docker-ps-server docker-find-client docker-monitor-server docker-run-server docker-run-client open-server open-client open-both open-monitor clean clean-all print-platform

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

run-client-interactive: $(CLIENT_BIN)
	./$(CLIENT_BIN) $(or $(HOST),$(DEFAULT_HOST)) $(or $(PORT),$(DEFAULT_PORT))

docker-stop-server:
	@if [ -z "$(CONTAINER)" ]; then \
		echo "실행 중인 Docker 컨테이너를 찾지 못했습니다." >&2; \
		exit 1; \
	fi
	@docker exec $(CONTAINER) bash -lc 'pid=$$(ss -ltnp | sed -n "s/.*:$(or $(PORT),$(DEFAULT_PORT)).*pid=\\([0-9][0-9]*\\).*/\\1/p" | head -n 1); if [ -n "$$pid" ]; then echo "기존 echo-server 종료: $$pid"; kill "$$pid"; fi'

docker-status-server:
	@if [ -z "$(CONTAINER)" ]; then \
		echo "실행 중인 Docker 컨테이너를 찾지 못했습니다." >&2; \
		exit 1; \
	fi
	docker exec $(CONTAINER) bash -lc 'ss -ltnp | grep :$(or $(PORT),$(DEFAULT_PORT)) || true'

docker-ps-server:
	@if [ -z "$(CONTAINER)" ]; then \
		echo "실행 중인 Docker 컨테이너를 찾지 못했습니다." >&2; \
		exit 1; \
	fi
	docker exec $(CONTAINER) bash -lc 'echo "[echo-server 프로세스]"; ps -o pid,ppid,pgid,stat,command -C echo-server.bin || true'

docker-find-client:
	@if [ -z "$(CONTAINER)" ]; then \
		echo "실행 중인 Docker 컨테이너를 찾지 못했습니다." >&2; \
		exit 1; \
	fi
	@if [ -z "$(CLIENT_PORT)" ]; then \
		echo "사용법: make docker-find-client CLIENT_PORT=<클라이언트포트> [PORT=<서버포트>]" >&2; \
		exit 1; \
	fi
	docker exec $(CONTAINER) bash -lc 'echo "[client port $(CLIENT_PORT) 처리 중인 child 검색]"; ss -tnp | grep ":$(or $(PORT),$(DEFAULT_PORT))" | grep ":$(CLIENT_PORT)" || true'

docker-monitor-server:
	@if [ -z "$(CONTAINER)" ]; then \
		echo "실행 중인 Docker 컨테이너를 찾지 못했습니다." >&2; \
		exit 1; \
	fi
	docker exec -it $(CONTAINER) bash -lc 'cd "$(CONTAINER_WORKDIR)" && bash ./monitor.sh $(or $(PORT),$(DEFAULT_PORT))'

docker-run-server:
	@if [ -z "$(CONTAINER)" ]; then \
		echo "실행 중인 Docker 컨테이너를 찾지 못했습니다." >&2; \
		exit 1; \
	fi
	@$(MAKE) docker-stop-server CONTAINER=$(CONTAINER) PORT=$(or $(PORT),$(DEFAULT_PORT))
	docker exec -it $(CONTAINER) bash -lc 'cd "$(CONTAINER_WORKDIR)" && make run-server PORT=$(or $(PORT),$(DEFAULT_PORT))'

docker-run-client:
	@if [ -z "$(CONTAINER)" ]; then \
		echo "실행 중인 Docker 컨테이너를 찾지 못했습니다." >&2; \
		exit 1; \
	fi
	docker exec -it $(CONTAINER) bash -lc 'cd "$(CONTAINER_WORKDIR)" && make run-client-interactive HOST=$(or $(HOST),$(DEFAULT_HOST)) PORT=$(or $(PORT),$(DEFAULT_PORT))'

open-server:
	@if ! command -v osascript >/dev/null 2>&1; then \
		echo "이 명령은 macOS 호스트 터미널에서 실행해야 합니다." >&2; \
		exit 1; \
	fi
	@if [ -z "$(CONTAINER)" ]; then \
		echo "실행 중인 Docker 컨테이너를 찾지 못했습니다." >&2; \
		exit 1; \
	fi
	osascript \
		-e 'tell application "Terminal" to do script "cd \"$(CURDIR)\" && make docker-run-server CONTAINER=$(CONTAINER) PORT=$(or $(PORT),$(DEFAULT_PORT))"'

open-client:
	@if ! command -v osascript >/dev/null 2>&1; then \
		echo "이 명령은 macOS 호스트 터미널에서 실행해야 합니다." >&2; \
		exit 1; \
	fi
	@if [ -z "$(CONTAINER)" ]; then \
		echo "실행 중인 Docker 컨테이너를 찾지 못했습니다." >&2; \
		exit 1; \
	fi
	@count=1; \
	while [ $$count -le $(CLIENT_COUNT) ]; do \
		osascript \
			-e 'tell application "Terminal" to do script "cd \"$(CURDIR)\" && make docker-run-client CONTAINER=$(CONTAINER) HOST=$(or $(HOST),$(DEFAULT_HOST)) PORT=$(or $(PORT),$(DEFAULT_PORT))"'; \
		count=$$((count + 1)); \
		sleep 0.2; \
	done

open-both:
	$(MAKE) open-server PORT=$(or $(PORT),$(DEFAULT_PORT))
	sleep 1
	$(MAKE) open-client HOST=$(or $(HOST),$(DEFAULT_HOST)) PORT=$(or $(PORT),$(DEFAULT_PORT)) CLIENT_COUNT=$(CLIENT_COUNT)

open-monitor:
	@if ! command -v osascript >/dev/null 2>&1; then \
		echo "이 명령은 macOS 호스트 터미널에서 실행해야 합니다." >&2; \
		exit 1; \
	fi
	@if [ -z "$(CONTAINER)" ]; then \
		echo "실행 중인 Docker 컨테이너를 찾지 못했습니다." >&2; \
		exit 1; \
	fi
	osascript \
		-e 'tell application "Terminal" to do script "cd \"$(CURDIR)\" && make docker-monitor-server CONTAINER=$(CONTAINER) PORT=$(or $(PORT),$(DEFAULT_PORT))"'

clean:
	rm -rf $(BUILD_DIR)

clean-all:
	rm -rf .build
