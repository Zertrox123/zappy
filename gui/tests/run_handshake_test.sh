#!/bin/sh
set -eu

GUI_BIN="${1:?gui binary path required}"
SCRIPT_DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
OUTPUT="$(mktemp)"

cleanup() {
    rm -f "$OUTPUT"
    if [ -n "${mock_pid:-}" ]; then
        kill "$mock_pid" 2>/dev/null || true
        wait "$mock_pid" 2>/dev/null || true
    fi
}
trap cleanup EXIT

python3 "$SCRIPT_DIR/mock_gui_server.py" >"$OUTPUT" &
mock_pid=$!

while [ ! -s "$OUTPUT" ]; do
    sleep 0.05
done
PORT="$(head -n 1 "$OUTPUT")"

ZAPPY_GUI_HEADLESS=1 "$GUI_BIN" -p "$PORT" -h 127.0.0.1
gui_rc=$?

wait "$mock_pid"
mock_rc=$?

if [ "$gui_rc" -ne 0 ] || [ "$mock_rc" -ne 0 ]; then
    echo "[run_handshake_test] gui_rc=$gui_rc mock_rc=$mock_rc" >&2
    exit 1
fi

exit 0
