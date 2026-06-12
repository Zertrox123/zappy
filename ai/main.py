#!/usr/bin/env python3

import sys
from pathlib import Path

if __name__ == "__main__":
    sys.path.insert(0, str(Path(__file__).resolve().parent))

from config import EXIT_USAGE, USAGE, ConfigParseError, parse_args  # noqa: E402


def main(argv: list[str] | None = None) -> int:
    args = argv if argv is not None else sys.argv
    if "--help" in args:
        sys.stdout.write(USAGE)
        return 0

    try:
        parse_args(args)
    except ConfigParseError as err:
        sys.stderr.write(f"{err}\n")
        return EXIT_USAGE

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
