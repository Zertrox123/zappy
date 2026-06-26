import sys
import unittest
from pathlib import Path
from unittest.mock import patch

DQN_DIR = Path(__file__).resolve().parents[1]
AI_DIR = DQN_DIR.parent.parent
sys.path.insert(0, str(DQN_DIR))
sys.path.insert(0, str(AI_DIR))

from config import EXIT_USAGE
from defaults import DEFAULT_EPISODES, DEFAULT_HOST, DEFAULT_PORT, DEFAULT_TEAM
from train import main as train_main


class TrainCliTests(unittest.TestCase):
    def test_help_exits_zero(self):
        self.assertEqual(train_main(["train.py", "--help"]), 0)

    def test_missing_args_exits_usage(self):
        self.assertEqual(train_main(["train.py"]), EXIT_USAGE)

    def test_valid_args_accepted(self):
        with patch("train.train_ai") as train_mock:
            rc = train_main(
                [
                    "train.py",
                    "-p",
                    str(DEFAULT_PORT),
                    "-n",
                    DEFAULT_TEAM,
                    "-h",
                    DEFAULT_HOST,
                    "--episodes",
                    "2",
                ]
            )
        self.assertEqual(rc, 0)
        train_mock.assert_called_once()
        cfg, episodes = train_mock.call_args[0]
        self.assertEqual(cfg.port, DEFAULT_PORT)
        self.assertEqual(cfg.team_name, DEFAULT_TEAM)
        self.assertEqual(cfg.hostname, DEFAULT_HOST)
        self.assertEqual(episodes, 2)

    def test_default_episodes(self):
        with patch("train.train_ai") as train_mock:
            train_main(
                [
                    "train.py",
                    "-p",
                    str(DEFAULT_PORT),
                    "-n",
                    DEFAULT_TEAM,
                    "-h",
                    DEFAULT_HOST,
                ]
            )
        _cfg, episodes = train_mock.call_args[0]
        self.assertEqual(episodes, DEFAULT_EPISODES)


if __name__ == "__main__":
    unittest.main()
