import re
import socket

import numpy as np


class ZappyEnv:
    def __init__(self, host, port, team):
        self.host = host
        self.port = port
        self.team_name = team
        self.sock = None
        self.buffer = ""
        self.actions = [
            "Forward",
            "Right",
            "Left",
            "Take food",
            "Take linemate",
            "Take deraumere",
            "Take sibur",
        ]
        self.action_space_n = len(self.actions)
        self.state_dim = 8

    def count_in_inventory(self, inv_text, resource):
        match = re.search(rf"{resource} (\d+)", inv_text)
        if match:
            return int(match.group(1))
        return 0

    def count_on_tile(self, tile, resource):
        tokens = [part.strip() for part in tile.split() if part.strip()]
        return tokens.count(resource)

    def food_urgency(self, food_count):
        return max(0.0, 1.0 - food_count / 5.0)

    def survival_score(self, food_count):
        return min(food_count * 126 / 1260.0, 1.0)

    def connect(self):
        if self.sock:
            self.sock.close()
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.connect((self.host, self.port))
        self.buffer = ""

    def receive_raw(self):
        while "\n" not in self.buffer:
            data = self.sock.recv(4096).decode()
            if not data:
                raise ConnectionError("Server closed the connection.")
            self.buffer += data
        line, self.buffer = self.buffer.split("\n", 1)
        return line.strip()

    def receive(self):
        while True:
            line = self.receive_raw()
            if (
                line.startswith("message")
                or line.startswith("eject")
                or line.startswith("Current")
                or line.startswith("Elevation")
            ):
                continue
            return line

    def send(self, msg):
        self.sock.sendall(f"{msg}\n".encode())

    def reset(self):
        self.connect()
        assert self.receive_raw() == "WELCOME", "No WELCOME received"
        self.send(self.team_name)
        slots = self.receive_raw()
        if slots == "ko":
            raise ValueError("Invalid team or no slots left.")
        self.receive_raw()
        state = self.get_state()
        if state is None:
            return np.zeros(self.state_dim)
        return state

    def compute_reward(self, command, response, food_count):
        reward = -1.0
        reward -= self.food_urgency(food_count) * 3.0
        if response == "ko":
            reward -= 5.0
        elif command == "Take food" and response == "ok":
            reward += 25.0
        elif command == "Take linemate" and response == "ok":
            reward += 10.0
        return reward

    def step(self, action_index):
        command = self.actions[action_index]
        self.send(command)
        response = self.receive()
        if response == "dead":
            return np.zeros(self.state_dim), -100.0, True
        next_state = self.get_state()
        if next_state is None:
            return np.zeros(self.state_dim), -100.0, True
        food_count = int(next_state[0])
        reward = self.compute_reward(command, response, food_count)
        return next_state, reward, False

    def get_state(self):
        inv_text = ""
        try:
            self.send("Inventory")
            inv_text = self.receive()
            if inv_text == "dead":
                return None
            self.send("Look")
            look = self.receive()
            if look == "dead":
                return None
            food_count = self.count_in_inventory(inv_text, "food")
            linemate_count = self.count_in_inventory(inv_text, "linemate")
            deraumere_count = self.count_in_inventory(inv_text, "deraumere")
            sibur_count = self.count_in_inventory(inv_text, "sibur")
            if look.startswith("[") and look.endswith("]"):
                tile = look.strip("[]").split(",")[0]
            else:
                tile = ""
            food_on_tile = self.count_on_tile(tile, "food")
            linemate_on_tile = self.count_on_tile(tile, "linemate")
            urgency = self.food_urgency(food_count)
            survival = self.survival_score(food_count)
            state = np.array(
                [
                    food_count,
                    linemate_count,
                    deraumere_count,
                    sibur_count,
                    food_on_tile,
                    linemate_on_tile,
                    urgency,
                    survival,
                ],
                dtype=np.float32,
            )
            return state
        except Exception as err:
            print(f"[Warning] State parse error: {err} | inventory: {inv_text}")
            return None
