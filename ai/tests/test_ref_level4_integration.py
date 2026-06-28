import os
import platform
import subprocess
import sys
import tempfile
import time
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
REF_SERVER = ROOT / "tools" / "reference-server" / "zappy_server"
SCRIPT = ROOT / "scripts" / "run-level4-test.sh"


def ref_binary_exists() -> bool:
    return REF_SERVER.is_file() and os.access(REF_SERVER, os.X_OK)


@unittest.skipUnless(
    os.environ.get("ZAPPY_RUN_REF_INTEGRATION") == "1"
    and ref_binary_exists()
    and platform.system() == "Darwin",
    "set ZAPPY_RUN_REF_INTEGRATION=1 to run reference server integration test",
)
class RefLevel4IntegrationTests(unittest.TestCase):
    def test_dual_client_reaches_level_4(self) -> None:
        env = os.environ.copy()
        env["ZAPPY_TIMEOUT_SEC"] = env.get("ZAPPY_TIMEOUT_SEC", "900")
        result = subprocess.run(
            [str(SCRIPT)],
            cwd=ROOT,
            env=env,
            capture_output=True,
            text=True,
            timeout=int(env["ZAPPY_TIMEOUT_SEC"]) + 120,
        )
        self.assertEqual(
            result.returncode,
            0,
            msg=result.stderr + result.stdout,
        )


if __name__ == "__main__":
    unittest.main()
