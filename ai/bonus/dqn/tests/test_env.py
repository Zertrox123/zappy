import sys
import unittest
from pathlib import Path
from unittest.mock import MagicMock, patch

import numpy as np

DQN_DIR = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(DQN_DIR))

from defaults import DEFAULT_HOST, DEFAULT_PORT, DEFAULT_TEAM
from env import ZappyEnv


class ZappyEnvStateTests(unittest.TestCase):
    def setUp(self):
        self.env = ZappyEnv(DEFAULT_HOST, DEFAULT_PORT, DEFAULT_TEAM)

    @patch.object(ZappyEnv, "send")
    @patch.object(ZappyEnv, "receive")
    def test_get_state_parses_inventory_and_look(self, receive_mock, send_mock):
        receive_mock.side_effect = [
            "food 2, linemate 1, deraumere 0, sibur 0, mendiane 0, phiras 0, thystame 0",
            "[ food, linemate, player ]",
        ]
        state = self.env.get_state()
        self.assertIsNotNone(state)
        np.testing.assert_array_equal(
            state,
            np.array([2, 1, 0, 0, 1, 0, 0.6, 0.2], dtype=np.float32),
        )

    @patch.object(ZappyEnv, "send")
    @patch.object(ZappyEnv, "receive")
    def test_get_state_returns_none_when_dead(self, receive_mock, send_mock):
        receive_mock.return_value = "dead"
        self.assertIsNone(self.env.get_state())

    def test_step_applies_reward_for_successful_food_pickup(self):
        self.env.sock = MagicMock()
        state_after = np.array([3, 0, 0, 0, 0, 0, 0.4, 0.3], dtype=np.float32)
        with patch.object(self.env, "send"), patch.object(
            self.env, "receive", return_value="ok"
        ), patch.object(self.env, "get_state", return_value=state_after):
            _, reward, done = self.env.step(self.env.actions.index("Take food"))
        self.assertEqual(reward, 22.8)
        self.assertFalse(done)

    def test_step_penalizes_low_food_inventory(self):
        self.env.sock = MagicMock()
        state_after = np.array([0, 0, 0, 0, 0, 0, 1.0, 0.0], dtype=np.float32)
        with patch.object(self.env, "send"), patch.object(
            self.env, "receive", return_value="ok"
        ), patch.object(self.env, "get_state", return_value=state_after):
            _, reward, done = self.env.step(self.env.actions.index("Forward"))
        self.assertEqual(reward, -4.0)
        self.assertFalse(done)

    def test_food_urgency_increases_when_food_is_low(self):
        self.assertGreater(self.env.food_urgency(0), self.env.food_urgency(4))

    def test_count_on_tile_splits_tokens(self):
        self.assertEqual(self.env.count_on_tile("food linemate player", "food"), 1)
        self.assertEqual(self.env.count_on_tile("superfood food", "food"), 1)
        self.assertEqual(self.env.count_on_tile("linemate player", "food"), 0)


if __name__ == "__main__":
    unittest.main()
