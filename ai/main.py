import sys
from pathlib import Path
from typing import List, Optional

sys.path.insert(0, str(Path(__file__).resolve().parent))

from algo.runner import run
from config import EXIT_USAGE, USAGE, ConfigParseError, parse_args


def main(argv: Optional[List[str]] = None) -> int:
    if argv is not None:
        args = argv
    else:
        args = sys.argv
    if "--help" in args or "-help" in args:
        sys.stdout.write(USAGE)
        return 0

    try:
        config = parse_args(args)
    except ConfigParseError as err:
        sys.stderr.write(f"{err}\n")
        return EXIT_USAGE

    return run(config)


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except Exception as err:
        sys.stderr.write(f"[!] Erreur fatale : {err}\n")
        raise SystemExit(1)
