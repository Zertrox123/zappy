"""Map exploration when no specific goal is active."""

from algo.world.map_model import MapModel
from algo.world.player_state import PlayerState


import random


def decide_exploration_action(player: PlayerState, map_model: MapModel) -> str:
    del map_model
    if not player.last_look_tiles:
        return "Look"

    # Randomly look 10% of the time so we keep our vision somewhat fresh
    # while wandering, preventing bots from starving while walking past food.
    choices = ["Forward"] * 8 + ["Right", "Left", "Look"]
    return random.choice(choices)
