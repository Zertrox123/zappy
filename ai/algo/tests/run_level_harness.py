import argparse
import sys
from pathlib import Path

AI_DIR = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(AI_DIR))

from algo.tests.level_harness import HarnessConfig, resolve_server, run_level_harness


def main() -> int:
    parser = argparse.ArgumentParser(description="Run Zappy level integration harness")
    parser.add_argument("--target-level", type=int, default=4)
    parser.add_argument("--timeout", type=int, default=300)
    parser.add_argument("--port", type=int, default=4242)
    parser.add_argument("--host", default="127.0.0.1")
    parser.add_argument("--team", default="team")
    parser.add_argument("--clients", type=int, default=2)
    parser.add_argument("--leader-headstart", type=int, default=120)
    args = parser.parse_args()

    if resolve_server() is None:
        print("error: no zappy_server binary found", file=sys.stderr)
        return 1

    config = HarnessConfig(
        port=args.port,
        host=args.host,
        team=args.team,
        target_level=args.target_level,
        timeout_sec=args.timeout,
        leader_headstart_sec=args.leader_headstart,
        client_count=args.clients,
    )

    try:
        result = run_level_harness(config)
    except (FileNotFoundError, RuntimeError) as err:
        print(f"error: {err}", file=sys.stderr)
        return 1

    for name, log_path in result.logs.items():
        print(f"[harness] {name} log: {log_path}", file=sys.stderr)

    print(f"[harness] max_level={result.max_level}", file=sys.stderr)
    if result.passed:
        print(
            f"[harness] passed: reached level >= {config.target_level}",
            file=sys.stderr,
        )
        return 0

    print(
        f"[harness] failed: no client reached level >= {config.target_level}",
        file=sys.stderr,
    )
    return 1


if __name__ == "__main__":
    raise SystemExit(main())
