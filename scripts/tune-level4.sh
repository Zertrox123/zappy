#!/usr/bin/env sh
# Run level-4 integration test until N consecutive passes (default 3).
#
# Usage: ./scripts/tune-level4.sh [passes]

set -eu

ROOT="$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)"
TARGET="${1:-3}"
TIMEOUT_SEC="${ZAPPY_TIMEOUT_SEC:-900}"
export ZAPPY_TIMEOUT_SEC="$TIMEOUT_SEC"

PASS_STREAK=0
RUN=0

while [ "$PASS_STREAK" -lt "$TARGET" ]; do
    RUN=$((RUN + 1))
    echo "[tune-level4] run $RUN (streak $PASS_STREAK/$TARGET)" >&2
    if "$ROOT/scripts/run-level4-test.sh"; then
        PASS_STREAK=$((PASS_STREAK + 1))
        echo "[tune-level4] pass $PASS_STREAK/$TARGET" >&2
    else
        PASS_STREAK=0
        echo "[tune-level4] failed, streak reset" >&2
    fi
done

echo "[tune-level4] done: $TARGET consecutive passes" >&2
