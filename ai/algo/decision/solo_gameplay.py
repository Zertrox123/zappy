"""Solo gameplay for level 1 to level 2."""

from algo.actions.exploration import decide_exploration_action
from algo.actions.navigation import enqueue_path_to_target
from algo.actions.resource_gathering import find_resource_in_vision
from algo.actions.survival import decide_survival_action
from algo.world.map_model import MapModel
from algo.world.player_state import PlayerState

SOLO_FOOD_THRESHOLD = 5


def _path_to_nearest_resource(
    player: PlayerState,
    map_model: MapModel,
    resource: str,
) -> str | None:
    target = map_model.find_nearest_tile(
        player.position,
        lambda _pos, objects: resource in objects,
    )
    if target is None:
        return None
    return enqueue_path_to_target(player, map_model, target)


def decide_solo_level_one_action(
    player: PlayerState,
    map_model: MapModel,
) -> str | None:
    if player.level >= 2:
        return None

    current_tile = map_model.tiles.get(player.position, [])
    if player.food_count() <= SOLO_FOOD_THRESHOLD and "food" in current_tile:
        return "Take food"

    if "linemate" in current_tile:
        if player.count_players_on_tile(current_tile) == 1:
            return "Incantation"
        return "Take linemate"

    if player.inventory.get("linemate", 0) > 0:
        if player.count_players_on_tile(current_tile) == 1:
            return "Set linemate"
        return decide_exploration_action(player, map_model)

    stone_action = find_resource_in_vision(player, map_model, "linemate")
    if stone_action is not None:
        return stone_action

    map_action = _path_to_nearest_resource(player, map_model, "linemate")
    if map_action is not None:
        return map_action

    if player.food_count() <= SOLO_FOOD_THRESHOLD:
        survival = decide_survival_action(player, map_model)
        if survival is not None:
            return survival

    return decide_exploration_action(player, map_model)
