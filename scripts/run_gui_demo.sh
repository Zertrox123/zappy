#!/bin/sh
set -eu

ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"
HOST="${1:-127.0.0.1}"
PORT="${2:-4242}"
TEAM="${3:-name1}"
REF="${ZAPPY_REF_SERVER:-/Users/karma/Downloads/Zappy v3.0.1/macos/zappy_server}"

cleanup() {
    kill "$SERVER_PID" "$GUI_PID" "$AI1_PID" "$AI2_PID" 2>/dev/null || true
}
trap cleanup EXIT INT TERM

if [ ! -x "$REF" ]; then
    echo "reference server not found: $REF" >&2
    echo "set ZAPPY_REF_SERVER or build ./zappy_server" >&2
    exit 1
fi

"$REF" -p "$PORT" -x 10 -y 10 -n "$TEAM" -c 6 -f 100 --auto-start on --display-eggs true &
SERVER_PID=$!
sleep 0.8

python3 "$ROOT/scripts/demo_ai.py" "$HOST" "$PORT" "$TEAM" Forward Right Forward Left &
AI1_PID=$!
python3 "$ROOT/scripts/demo_ai.py" "$HOST" "$PORT" "$TEAM" Forward Forward Right Right &
AI2_PID=$!
sleep 0.5

make -C "$ROOT/gui" -s re
"$ROOT/zappy_gui" -p "$PORT" -h "$HOST" &
GUI_PID=$!

wait "$GUI_PID"
