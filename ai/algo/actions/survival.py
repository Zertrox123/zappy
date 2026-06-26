"""Food survival: take visible food or path toward known food tiles."""

from algo.actions.navigation import navigate_toward_vision_index
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

    return None
