#!/usr/bin/env sh
# Dual-client level-4 integration test against the Epitech reference server.
#
# macOS arm64 only (tools/reference-server/zappy_server). Not for CI.
#
# Usage:
#   ./scripts/run-level4-test.sh
#
# Environment:
#   PORT=4242 TEAM=team HOST=localhost ZAPPY_TIMEOUT_SEC=900

set -eu

ROOT="$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)"
PORT="${PORT:-4242}"
TEAM="${TEAM:-team}"
HOST="${HOST:-localhost}"
TIMEOUT_SEC="${ZAPPY_TIMEOUT_SEC:-900}"
WIDTH="${WIDTH:-10}"
HEIGHT="${HEIGHT:-10}"
FREQ="${FREQ:-50}"
LEADER_HEADSTART_SEC="${LEADER_HEADSTART_SEC:-120}"
export PORT TEAM HOST ZAPPY_TIMEOUT_SEC="$TIMEOUT_SEC" WIDTH HEIGHT FREQ

cleanup() {
    PIDS="$(lsof -ti :"$PORT" 2>/dev/null || true)"
    if [ -n "$PIDS" ]; then
        kill $PIDS 2>/dev/null || true
    fi
    if [ -n "${FOLLOWER_PID:-}" ]; then
        kill "$FOLLOWER_PID" 2>/dev/null || true
    fi
    if [ -n "${LEADER_PID:-}" ]; then
        kill "$LEADER_PID" 2>/dev/null || true
    fi
}
trap cleanup EXIT INT TERM
cleanup

"$ROOT/scripts/start-ref-server.sh" --bg >/dev/null

READY=0
for _ in 1 2 3 4 5 6 7 8 9 10; do
    if nc -z "$HOST" "$PORT" 2>/dev/null; then
        READY=1
        break
    fi
    sleep 0.3
done

if [ "$READY" -eq 0 ]; then
    echo "[level4-test] error: server not listening on $HOST:$PORT" >&2
    exit 1
fi

echo "[level4-test] building zappy_ai..."
make -C "$ROOT/ai" >/dev/null

LOG_DIR="$(mktemp -d)"
FOLLOWER_LOG="$LOG_DIR/follower.log"
LEADER_LOG="$LOG_DIR/leader.log"

echo "[level4-test] launching leader (background, ${LEADER_HEADSTART_SEC}s head start)..."
ZAPPY_ROLE=leader "$ROOT/zappy_ai" -p "$PORT" -n "$TEAM" -h "$HOST" >"$LEADER_LOG" 2>&1 &
LEADER_PID=$!

sleep "$LEADER_HEADSTART_SEC"

echo "[level4-test] launching follower (background)..."
ZAPPY_ROLE=follower "$ROOT/zappy_ai" -p "$PORT" -n "$TEAM" -h "$HOST" >"$FOLLOWER_LOG" 2>&1 &
FOLLOWER_PID=$!

DEADLINE=$(( $(date +%s) + TIMEOUT_SEC ))
PASS=0

while [ "$(date +%s)" -lt "$DEADLINE" ]; do
    if ! kill -0 "$FOLLOWER_PID" 2>/dev/null && ! kill -0 "$LEADER_PID" 2>/dev/null; then
        break
    fi

    for LOG in "$FOLLOWER_LOG" "$LEADER_LOG"; do
        if grep -qE 'level=[4-9].*alive=True' "$LOG" 2>/dev/null; then
            PASS=1
            break
        fi
    done

    if [ "$PASS" -eq 1 ]; then
        break
    fi
    sleep 2
done

kill "$FOLLOWER_PID" 2>/dev/null || true
kill "$LEADER_PID" 2>/dev/null || true
wait "$FOLLOWER_PID" 2>/dev/null || true
wait "$LEADER_PID" 2>/dev/null || true

echo "[level4-test] follower log:" >&2
tail -n 3 "$FOLLOWER_LOG" >&2 || true
echo "[level4-test] leader log:" >&2
tail -n 3 "$LEADER_LOG" >&2 || true

if [ "$PASS" -eq 1 ]; then
    echo "[level4-test] passed: reached level >= 4" >&2
    exit 0
fi

echo "[level4-test] failed: no client reached level >= 4 within ${TIMEOUT_SEC}s" >&2
exit 1
