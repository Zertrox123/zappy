#!/bin/sh
set -e

root="$(cd "$(dirname "$0")" && pwd)"
cd "$root"

missing=0

if ! command -v rustfmt >/dev/null 2>&1; then
    echo "Installing rustfmt..."
    rustup component add rustfmt
fi

if ! command -v clang-format >/dev/null 2>&1; then
    if command -v brew >/dev/null 2>&1; then
        echo "Installing clang-format..."
        brew install clang-format
    else
        echo "clang-format not found. Install it with your package manager." >&2
        missing=1
    fi
fi

if ! command -v black >/dev/null 2>&1; then
    if command -v brew >/dev/null 2>&1; then
        echo "Installing black..."
        brew install black
    else
        echo "black not found. Install it with: python3 -m pip install --user black" >&2
        missing=1
    fi
fi

if [ "$missing" -ne 0 ]; then
    exit 1
fi

echo "Formatters ready:"
echo "  rustfmt:       $(command -v rustfmt)"
echo "  clang-format:  $(command -v clang-format)"
echo "  black:         $(command -v black)"
echo "Run: make format"
