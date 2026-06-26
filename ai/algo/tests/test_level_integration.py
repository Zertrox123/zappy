import os
import sys
import unittest
from pathlib import Path

AI_DIR = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(AI_DIR))

from algo.tests.level_harness import HarnessConfig, resolve_server, run_level_harness


@unittest.skipUnless(
    os.environ.get("ZAPPY_RUN_LIVE") == "1" and resolve_server() is not None,
    "set ZAPPY_RUN_LIVE=1 and provide zappy_server to run live integration",
)
class LevelIntegrationTests(unittest.TestCase):
    def test_dual_client_reaches_target_level(self) -> None:
        timeout = int(os.environ.get("ZAPPY_TIMEOUT_SEC", "900"))
        target = int(os.environ.get("ZAPPY_TARGET_LEVEL", "4"))
        port = int(os.environ.get("PORT", "4242"))
        team = os.environ.get("TEAM", "team")
        host = os.environ.get("HOST", "127.0.0.1")
        headstart = int(os.environ.get("LEADER_HEADSTART_SEC", "120"))

        config = HarnessConfig(
            port=port,
            host=host,
            team=team,
            target_level=target,
            timeout_sec=timeout,
            leader_headstart_sec=headstart,
            client_count=2,
        )
        result = run_level_harness(config)
        self.assertGreaterEqual(
            result.max_level,
            target,
            msg=f"max_level={result.max_level} logs={result.logs}",
        )


if __name__ == "__main__":
    unittest.main()
