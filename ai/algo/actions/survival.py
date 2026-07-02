"""Food survival: take visible food, or path toward the nearest known food."""

from algo.actions.navigation import (
    enqueue_path_to_target,
    navigate_toward_vision_index,
)
from algo.world.map_model import MapModel
from algo.world.player_state import PlayerState


def decide_survival_action(
    player: PlayerState,
    map_model: MapModel,
) -> str | None:
    current_tile = map_model.tiles.get(player.position, [])
    if player.food_count() <= 15 and "food" in current_tile:
        return "Take food"

    for look_tile in player.last_look_tiles:
        if "food" not in look_tile.objects:
            continue
        if look_tile.index == 0:
            return "Take food"

        move = navigate_toward_vision_index(player, look_tile.index, map_model)
        if move is not None:
            return move

    food_target = map_model.find_nearest_tile(
        player.position, lambda _pos, objects: "food" in objects
    )
    if food_target is not None and food_target != player.position:
        move = enqueue_path_to_target(player, map_model, food_target)
        if move is not None:
            return move

    return None
