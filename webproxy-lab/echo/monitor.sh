#!/usr/bin/env bash

set -u

PORT="${1:-9000}"
REFRESH_SECONDS="${REFRESH_SECONDS:-1}"

find_listen_pid() {
  ss -ltnp 2>/dev/null |
    sed -n "s/.*:${PORT} .*pid=\([0-9][0-9]*\).*/\1/p" |
    head -n 1
}

render_header() {
  local root_pid="$1"

  printf '============================================================\n'
  printf ' Echo Monitor\n'
  printf '============================================================\n'
  printf ' time        : %s\n' "$(date '+%Y-%m-%d %H:%M:%S')"
  printf ' target port : %s\n' "${PORT}"
  if [ -n "${root_pid}" ]; then
    printf ' listen pid  : %s\n' "${root_pid}"
  else
    printf ' listen pid  : 없음\n'
  fi
  printf ' refresh     : %ss\n' "${REFRESH_SECONDS}"
  printf '\n'
}

render_help() {
  printf '[힌트]\n'
  printf ' - 클라이언트 종료: exit / Ctrl+D / Ctrl+C\n'
  printf ' - 특정 클라이언트 찾기: make docker-find-client CLIENT_PORT=<포트> PORT=%s\n' "${PORT}"
  printf '\n'
}

render_listen_status() {
  printf '[LISTEN 상태]\n'
  ss -ltnp 2>/dev/null | awk -v port="${PORT}" '$4 ~ ":" port "$" {print}'
  printf '\n'
}

render_processes() {
  local root_pid="$1"

  printf '[부모/자식 프로세스]\n'
  if [ -z "${root_pid}" ]; then
    printf ' 대상 포트에서 리슨 중인 echo-server.bin 을 찾지 못했습니다.\n\n'
    return
  fi

  printf "%-8s %-8s %-8s %-8s %-10s %-8s %s\n" \
    "ROLE" "PID" "PPID" "STAT" "ELAPSED" "PGID" "COMMAND"
  ps -o pid=,ppid=,pgid=,stat=,etime=,command= -C echo-server.bin 2>/dev/null |
    awk -v root="${root_pid}" '
      {
        role = "";
        order = 9;
        if ($1 == root) {
          role = "parent";
          order = 0;
        } else if ($2 == root) {
          role = "child";
          order = 1;
        } else {
          next;
        }

        printf "%d\t%-8s %-8s %-8s %-8s %-10s %-8s ",
               order, role, $1, $2, $4, $5, $3;
        for (i = 6; i <= NF; i++) {
          if (i > 6) {
            printf " ";
          }
          printf "%s", $i;
        }
        printf "\n";
      }
    ' |
    sort -n |
    cut -f2-
  printf '\n'
}

render_connections() {
  printf '[현재 연결]\n'
  if ! ss -tnp 2>/dev/null | awk -v port="${PORT}" '$4 ~ ":" port "$" { found = 1 } END { exit(found ? 0 : 1) }'; then
    printf ' 현재 %s 포트에 연결된 클라이언트가 없습니다.\n\n' "${PORT}"
    return
  fi

  printf "%-12s %-24s %-24s %-10s %s\n" \
    "STATE" "LOCAL" "PEER" "PID" "PROCESS"
  ss -tnp 2>/dev/null |
    awk -v port="${PORT}" '
      $4 ~ ":" port "$" {
        pid = "-";
        proc = "-";
        peer_sort = $5;
        sub(/^.*:/, "", peer_sort);
        if (match($6, /pid=[0-9]+/)) {
          pid = substr($6, RSTART + 4, RLENGTH - 4);
        }
        if (match($6, /echo-server\.bin/)) {
          proc = "echo-server.bin";
        }
        printf "%s\t%-12s %-24s %-24s %-10s %s\n",
               peer_sort, $1, $4, $5, pid, proc;
      }
    ' |
    sort -n |
    cut -f2-
  printf '\n'
}

while true; do
  root_pid="$(find_listen_pid)"
  if [ -n "${TERM:-}" ] && [ "${TERM}" != "dumb" ]; then
    clear
  fi
  render_header "${root_pid}"
  render_help
  render_listen_status
  render_processes "${root_pid}"
  render_connections
  sleep "${REFRESH_SECONDS}"
done
