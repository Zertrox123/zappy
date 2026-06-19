#!/usr/bin/env sh
# Start the Epitech reference zappy_server with AI-friendly defaults.
#
# macOS arm64 binary (Epitech Zappy v3.0.1). Override path with ZAPPY_REF_SERVER.
#
# Environment overrides:
#   PORT=4242 TEAM=team WIDTH=10 HEIGHT=10 CLIENTS=6 FREQ=100
#
# Usage:
#   ./scripts/start-ref-server.sh          # foreground (terminal stdin)
#   ./scripts/start-ref-server.sh --bg     # background (prints server PID)

set -eu

ROOT="$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)"
SERVER="${ZAPPY_REF_SERVER:-$ROOT/tools/reference-server/zappy_server}"

PORT="${PORT:-4242}"
TEAM="${TEAM:-team}"
WIDTH="${WIDTH:-10}"
HEIGHT="${HEIGHT:-10}"
CLIENTS="${CLIENTS:-6}"
FREQ="${FREQ:-100}"
AUTO_START="${AUTO_START:-on}"
DISPLAY_EGGS="${DISPLAY_EGGS:-false}"

if [ ! -x "$SERVER" ]; then
    echo "error: reference server not found or not executable: $SERVER" >&2
    exit 1
fi

ARGS="-p $PORT -x $WIDTH -y $HEIGHT -n $TEAM -c $CLIENTS -f $FREQ --auto-start $AUTO_START --display-eggs $DISPLAY_EGGS"

if [ "${1:-}" = "--bg" ]; then
    tail -f /dev/null | "$SERVER" $ARGS &
    echo $!
    exit 0
fi

echo "[ref-server] port=$PORT team=$TEAM map=${WIDTH}x${HEIGHT} clients=$CLIENTS freq=$FREQ auto-start=$AUTO_START"
exec "$SERVER" $ARGS
