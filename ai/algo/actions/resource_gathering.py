"""Stone gathering from vision and map memory."""

from algo.actions.navigation import enqueue_path_to_target
from algo.actions.resource_assignments import missing_stones_in_inventory
from algo.world.elevation_rules import ELEVATION_REQUIREMENTS
from algo.world.map_model import MapModel, Position, VISION_OFFSETS, rotate_offset
from algo.world.player_state import PlayerState


def find_resource_in_vision(
    player: PlayerState,
    map_model: MapModel,
    resource: str,
    *,
    allow_take_on_current_tile: bool = True,
) -> str | None:
    if allow_take_on_current_tile:
        current_tile = map_model.tiles.get(player.position, [])
        if resource in current_tile:
            return f"Take {resource}"

    for look_tile in player.last_look_tiles:
        if resource not in look_tile.objects:
            continue
        if look_tile.index == 0 and allow_take_on_current_tile:
            return f"Take {resource}"
        if look_tile.index <= 0 or look_tile.index >= len(VISION_OFFSETS):
            continue
        dx, dy = VISION_OFFSETS[look_tile.index]
        rdx, rdy = rotate_offset(dx, dy, player.orientation)
        target = Position(
            (player.position.x + rdx) % map_model.width,
            (player.position.y + rdy) % map_model.height,
        )
        move = enqueue_path_to_target(player, map_model, target)
        if move is not None:
            return move
    return None


def gather_assigned_stones(
    player: PlayerState,
    map_model: MapModel,
    target_level: int,
) -> str | None:
    next_level = player.level + 1
    if next_level > target_level:
        return None

    missing = missing_stones_in_inventory(player.role, next_level, player.inventory)
    if not missing:
        return None

    for resource in missing:
        action = find_resource_in_vision(player, map_model, resource)
        if action is not None:
            return action

    for resource in missing:
        target = map_model.find_nearest_tile(
            player.position,
            lambda _pos, objects: resource in objects,
        )
        if target is None:
            continue
        move = enqueue_path_to_target(player, map_model, target)
        if move is not None:
            return move

    return None


def place_stones_from_inventory(
    player: PlayerState,
    map_model: MapModel,
    target_level: int,
    at_position: Position,
) -> str | None:
    """Place stones that the tile still needs for incantation.

    Uses ELEVATION_REQUIREMENTS (total needed on tile) and checks both
    Look data and our own placement tracking so we don't duplicate stones
    that another player already placed.
    """
    del map_model
    if target_level not in ELEVATION_REQUIREMENTS:
        return None
    _, resources = ELEVATION_REQUIREMENTS[target_level]

    # What is currently visible on tile 0 from the last Look?
    tile_objects: list[str] = []
    for look_tile in player.last_look_tiles:
        if look_tile.index == 0:
            tile_objects = list(look_tile.objects)
            break

    for resource, needed in resources.items():
        on_tile_visible = sum(1 for obj in tile_objects if obj == resource)
        on_tile_placed = player.ceremony_stones_on_tile.get(resource, 0)
        on_tile = max(on_tile_visible, on_tile_placed)
        if on_tile < needed and player.inventory.get(resource, 0) > 0:
            return f"Set {resource}"
    return None
