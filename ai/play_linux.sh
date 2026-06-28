#!/bin/bash
# Usage: ./scripts/play_linux.sh [N_AI]
# Runs the locally compiled Zappy server (Linux) and AI clients against it.
# Stop everything with Ctrl-C.

set -eu

ROOT="$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)"
SERVER="$ROOT/zappy_server"

PORT="${PORT:-4242}"
TEAM="${TEAM:-team}"
HOST="localhost"
WIDTH="${WIDTH:-10}"
HEIGHT="${HEIGHT:-10}"
CLIENTS="${CLIENTS:-6}"
FREQ="${FREQ:-100}"
AUTO_START="${AUTO_START:-on}"
N_AI="${1:-1}"

if [ ! -x "$SERVER" ]; then
    echo "error: linux server not found or not executable: $SERVER" >&2
    echo "Please compile the server first (make zappy_server)." >&2
    exit 1
fi

SERVER_LOG="$(mktemp -t zappy_server.XXXXXX)"
FIFO="$(mktemp -u -t zappy_stdin.XXXXXX)"
mkfifo "$FIFO"

AI_PIDS=""
SERVER_PID=""
HOLD_PID=""

cleanup() {
    [ -n "$AI_PIDS" ] && kill $AI_PIDS 2>/dev/null || true
    [ -n "$SERVER_PID" ] && kill "$SERVER_PID" 2>/dev/null || true
    [ -n "$HOLD_PID" ] && kill "$HOLD_PID" 2>/dev/null || true
    PIDS="$(lsof -ti :"$PORT" 2>/dev/null || true)"
    [ -n "$PIDS" ] && kill $PIDS 2>/dev/null || true
    rm -f "$FIFO" "$SERVER_LOG" 2>/dev/null || true
}
trap cleanup EXIT INT TERM

# Free the port if a previous run left something listening.
EXISTING="$(lsof -ti :"$PORT" 2>/dev/null || true)"
[ -n "$EXISTING" ] && kill $EXISTING 2>/dev/null || true

# Hold the server's stdin open so it does not exit on EOF.
sleep 2147483647 > "$FIFO" &
HOLD_PID=$!
export ZAPPY_TARGET_LEVEL=8

echo "[play_linux] starting server ($SERVER) on $HOST:$PORT (team=$TEAM, map=${WIDTH}x${HEIGHT}, freq=$FREQ)"
"$SERVER" -p "$PORT" -x "$WIDTH" -y "$HEIGHT" -n "$TEAM" -c "$CLIENTS" \
    -f "$FREQ" < "$FIFO" > "$SERVER_LOG" 2>&1 &
SERVER_PID=$!

READY=0
for _ in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15; do
    if nc -z "$HOST" "$PORT" 2>/dev/null; then
        READY=1
        break
    fi
    if [ -n "$SERVER_PID" ] && ! kill -0 "$SERVER_PID" 2>/dev/null; then
        break
    fi
    sleep 0.2
done

if [ "$READY" -eq 0 ]; then
    echo "[play_linux] error: server not listening on $HOST:$PORT" >&2
    if [ -n "$SERVER_PID" ]; then
        echo "[play_linux] server log:" >&2
        cat "$SERVER_LOG" >&2 || true
    fi
    exit 1
fi
echo "[play_linux] server is up"

echo "[play_linux] building zappy_ai..."
make -C "$ROOT/ai" >/dev/null

echo "[play_linux] launching $N_AI AI client(s) on team '$TEAM'..."
i=1
while [ "$i" -le "$N_AI" ]; do
    ROLE="${ZAPPY_ROLE:-}"
    if [ -z "$ROLE" ]; then
        if [ "$N_AI" -eq 1 ]; then
            ROLE="single"
        elif [ "$i" -eq 1 ]; then
            ROLE="leader"
        else
            ROLE="follower"
        fi
    fi

    echo "[play_linux] launching ai-$i (role=$ROLE)"
    ZAPPY_ROLE="$ROLE" ZAPPY_SLOT="$i" "$ROOT/zappy_ai" -p "$PORT" -n "$TEAM" -h "$HOST" 2>&1 \
        | while IFS= read -r line; do
            printf '[ai-%s] %s\n' "$i" "$line"
        done &
    AI_PIDS="$AI_PIDS $!"
    i=$((i + 1))
done

echo "[play_linux] running - press Ctrl-C to stop"
wait
