#!/usr/bin/env python3

from __future__ import annotations

import sys
from pathlib import Path

if __name__ == "__main__":
    sys.path.insert(0, str(Path(__file__).resolve().parent))

from config import (  # noqa: E402
    EXIT_USAGE,
    USAGE,
    AiConfig,
    ConfigParseError,
    HelpRequested,
    parse_args,
)


def print_usage() -> None:
    sys.stdout.write(USAGE)


def run_ai(config: AiConfig) -> int:
    _ = config
    return 0


def main(argv: list[str] | None = None) -> int:
    args = argv if argv is not None else sys.argv
    try:
        config = parse_args(args)
    except HelpRequested:
        print_usage()
        return 0
    except ConfigParseError as err:
        sys.stderr.write(f"{err}\n")
        return EXIT_USAGE

    return run_ai(config)


if __name__ == "__main__":
    raise SystemExit(main())
