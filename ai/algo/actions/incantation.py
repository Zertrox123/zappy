"""Incantation preconditions and elevation checks."""

from algo.world.elevation_rules import ELEVATION_REQUIREMENTS
from algo.world.map_model import MapModel
from algo.world.player_state import PlayerState


def _current_tile_objects(player: PlayerState) -> list[str]:
    for look_tile in player.last_look_tiles:
        if look_tile.index == 0:
            return look_tile.objects
    return []


def stones_satisfied_on_tile(
    player: PlayerState,
    map_model: MapModel,
    level: int,
) -> bool:
    del map_model
    if level not in ELEVATION_REQUIREMENTS:
        return False
    _, resources = ELEVATION_REQUIREMENTS[level]
    tile_objects = _current_tile_objects(player)
    if not tile_objects:
        return False
    for name, needed in resources.items():
        if player.count_on_tile(tile_objects, name) < needed:
            return False
    return True


def partner_is_present(
    player: PlayerState,
    map_model: MapModel,
    next_level: int,
    target_level: int = 8,
) -> bool:
    """Check whether enough players are on tile 0 via Look data."""
    del map_model
    if next_level <= 2:
        return True

    required = player.players_required(next_level)

    for look_tile in player.last_look_tiles:
        if look_tile.index == 0:
            count = player.count_players_on_tile(look_tile.objects)
            return count >= required
    return False


def can_start_incantation(
    player: PlayerState,
    map_model: MapModel,
    next_level: int,
    target_level: int = 8,
) -> bool:
    if next_level not in ELEVATION_REQUIREMENTS:
        return False
    if player.food_count() < 5:
        return False
    if not partner_is_present(player, map_model, next_level, target_level):
        return False
    return stones_satisfied_on_tile(player, map_model, next_level)


def ceremony_requirements_satisfied(
    player: PlayerState,
    map_model: MapModel,
    next_level: int,
) -> bool:
    return can_start_incantation(player, map_model, next_level)
