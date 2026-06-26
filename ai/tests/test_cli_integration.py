import subprocess
import sys
import unittest
from pathlib import Path

from config import EXIT_USAGE, USAGE

AI_DIR = Path(__file__).resolve().parent.parent
REPO_ROOT = AI_DIR.parent


def run_main(*args: str) -> subprocess.CompletedProcess[str]:
    return subprocess.run(
        [sys.executable, "main.py", *args],
        cwd=AI_DIR,
        capture_output=True,
        text=True,
        check=False,
    )


def run_binary(*args: str) -> subprocess.CompletedProcess[str]:
    binary = REPO_ROOT / "zappy_ai"
    if not binary.exists():
        subprocess.run(["make", "-C", str(AI_DIR)], check=True, capture_output=True)
    return subprocess.run(
        [str(binary), *args],
        cwd=REPO_ROOT,
        capture_output=True,
        text=True,
        check=False,
    )


class MainIntegrationTests(unittest.TestCase):
    def test_help_flag_prints_usage_and_exits_zero(self) -> None:
        output = run_main("--help")
        self.assertEqual(output.returncode, 0)
        self.assertEqual(output.stdout, USAGE)
        self.assertEqual(output.stderr, "")

    def test_no_arguments_exits_with_usage_code(self) -> None:
        output = run_main()
        self.assertEqual(output.returncode, EXIT_USAGE)
        self.assertIn("missing required argument: -p", output.stderr)

    def test_invalid_port_exits_with_usage_code(self) -> None:
        output = run_main("-p", "0", "-n", "team")
        self.assertEqual(output.returncode, EXIT_USAGE)
        self.assertIn("invalid value for -p: 0", output.stderr)

    def test_valid_configuration_exits_zero_without_server(self) -> None:
        output = run_main("-p", "8080", "-n", "team", "-h", "127.0.0.1")
        self.assertEqual(output.returncode, 0)


class BinaryIntegrationTests(unittest.TestCase):
    def test_root_binary_help(self) -> None:
        output = run_binary("--help")
        self.assertEqual(output.returncode, 0)
        self.assertEqual(output.stdout, USAGE)

    def test_root_binary_valid_configuration(self) -> None:
        output = run_binary("-p", "4242", "-n", "team")
        self.assertEqual(output.returncode, 0)


if __name__ == "__main__":
    unittest.main()
