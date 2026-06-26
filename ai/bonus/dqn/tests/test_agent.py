import sys
import unittest
from pathlib import Path

import numpy as np

DQN_DIR = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(DQN_DIR))

from agent import DQNAgent


class DQNAgentTests(unittest.TestCase):
    def setUp(self):
        self.agent = DQNAgent(state_dim=8, action_dim=7)

    def test_select_action_returns_valid_index(self):
        state = np.zeros(8, dtype=np.float32)
        self.agent.epsilon = 1.0
        action = self.agent.select_action(state)
        self.assertGreaterEqual(action, 0)
        self.assertLess(action, self.agent.action_dim)

    def test_select_greedy_action_returns_valid_index(self):
        state = np.zeros(8, dtype=np.float32)
        action = self.agent.select_greedy_action(state)
        self.assertGreaterEqual(action, 0)
        self.assertLess(action, self.agent.action_dim)

    def test_load_and_save_round_trip(self):
        import tempfile
        from pathlib import Path

        state = np.zeros(8, dtype=np.float32)
        action_before = self.agent.select_greedy_action(state)
        with tempfile.TemporaryDirectory() as tmp:
            model_path = Path(tmp) / "model.pt"
            self.agent.save(str(model_path))
            other_agent = DQNAgent(state_dim=8, action_dim=7)
            other_agent.load(str(model_path))
            action_after = other_agent.select_greedy_action(state)
        self.assertEqual(action_before, action_after)

    def test_replay_skips_until_batch_is_full(self):
        state = np.zeros(8, dtype=np.float32)
        for _ in range(self.agent.batch_size - 1):
            self.agent.remember(state, 0, 1.0, state, False)
        epsilon_before = self.agent.epsilon
        self.agent.replay()
        self.assertEqual(self.agent.epsilon, epsilon_before)

    def test_replay_trains_when_memory_is_full(self):
        state = np.zeros(8, dtype=np.float32)
        for _ in range(self.agent.batch_size):
            self.agent.remember(state, 0, 1.0, state, False)
        epsilon_before = self.agent.epsilon
        self.agent.replay()
        self.assertLess(self.agent.epsilon, epsilon_before)


if __name__ == "__main__":
    unittest.main()
