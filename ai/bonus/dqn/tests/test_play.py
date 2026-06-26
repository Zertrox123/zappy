import sys
import unittest
from pathlib import Path
from unittest.mock import patch

import numpy as np

DQN_DIR = Path(__file__).resolve().parents[1]
AI_DIR = DQN_DIR.parent.parent
sys.path.insert(0, str(DQN_DIR))
sys.path.insert(0, str(AI_DIR))

from config import AiConfig
from defaults import DEFAULT_HOST, DEFAULT_PORT, DEFAULT_TEAM
from play import play_ai


class PlayAiTests(unittest.TestCase):
    def setUp(self):
        self.config = AiConfig(
            port=DEFAULT_PORT, team_name=DEFAULT_TEAM, hostname=DEFAULT_HOST
        )

    @patch("play.DQNAgent")
    @patch("play.ZappyEnv")
    def test_play_reconnects_after_death(self, env_cls, agent_cls):
        env = env_cls.return_value
        agent = agent_cls.return_value
        env.state_dim = 8
        env.action_space_n = 7
        env.reset.side_effect = [
            np.zeros(8, dtype=np.float32),
            np.zeros(8, dtype=np.float32),
            OSError("stop loop"),
        ]
        env.step.side_effect = [
            (np.zeros(8, dtype=np.float32), -1.0, True),
            (np.zeros(8, dtype=np.float32), -1.0, True),
        ]
        agent.select_greedy_action.return_value = 0

        with patch("builtins.print"):
            play_ai(self.config)

        self.assertGreaterEqual(env.reset.call_count, 2)
        self.assertEqual(agent.epsilon, 0.0)

    @patch("play.DQNAgent")
    @patch("play.ZappyEnv")
    def test_play_loads_model_when_provided(self, env_cls, agent_cls):
        env = env_cls.return_value
        agent = agent_cls.return_value
        env.state_dim = 8
        env.action_space_n = 7
        env.reset.side_effect = OSError("connection refused")

        with patch("builtins.print"):
            play_ai(self.config, model_path="/tmp/model.pt")

        agent.load.assert_called_once_with("/tmp/model.pt")

    @patch("play.ZappyEnv")
    def test_play_exits_on_connection_error(self, env_cls):
        env_cls.return_value.reset.side_effect = OSError("connection refused")

        with patch("builtins.print"):
            play_ai(self.config)


if __name__ == "__main__":
    unittest.main()
