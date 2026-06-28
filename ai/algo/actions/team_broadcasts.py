"""Apply incoming team broadcasts and ceremony stone accounting."""

from algo.actions.resource_assignments import stones_for_role
from algo.world.broadcast_messages import (
    TeamBroadcast,
    parse_regroup_message,
    parse_ready_message,
)
from algo.world.map_model import MapModel
from algo.world.player_state import LeaderInfo, PlayerState, Role


def apply_team_broadcasts(
    player: PlayerState,
    broadcasts: list[TeamBroadcast],
) -> None:
    for item in broadcasts:
        regroup_result = parse_regroup_message(item.message)
        if regroup_result is not None:
            if isinstance(regroup_result, tuple):
                level, sender_uuid = regroup_result
            else:
                level, sender_uuid = regroup_result, ""

            if level == player.level + 1:
                if (
                    player.role == Role.FOLLOWER
                    and player.leader_info is not None
                    and player.leader_info.level == level
                    and player.leader_info.direction == 0
                    and sender_uuid != player.leader_info.uuid
                    and sender_uuid != ""
                ):
                    continue

                player.leader_info = LeaderInfo(
                    direction=item.direction,
                    level=level,
                    uuid=sender_uuid,
                )
                player.ticks_since_leader = 0

                if item.direction == 0:
                    player.pending_moves.clear()

                if player.role == Role.LEADER:
                    if not sender_uuid or sender_uuid > player.uuid:
                        player.role = Role.FOLLOWER
                        player.pending_moves.clear()
            continue

        ready_level_result = parse_ready_message(item.message)
        if ready_level_result is not None:
            if isinstance(ready_level_result, tuple):
                ready_level, player_uuid = ready_level_result
                player.ready_level = ready_level
                if player.role == Role.LEADER and ready_level == player.level + 1:
                    player.ready_followers.add(player_uuid)
            else:
                player.ready_level = ready_level_result


def stones_on_tile(
    player: PlayerState,
    tile_objects: list[str],
    resource: str,
) -> int:
    return max(
        player.count_on_tile(tile_objects, resource),
        player.ceremony_stones_on_tile.get(resource, 0),
    )


def missing_stones_for_role(
    player: PlayerState,
    map_model: MapModel,
    next_level: int,
) -> dict[str, int]:
    """Stones still needed in inventory or on the ceremony tile."""
    assigned = stones_for_role(player.role, next_level)
    missing: dict[str, int] = {}

    if player.role == Role.FOLLOWER:
        for name, needed in assigned.items():
            have = player.inventory.get(name, 0)
            if have < needed:
                missing[name] = needed - have
        return missing

    rally = player.rally_position
    rally_tile: list[str] = map_model.tiles.get(rally, []) if rally else []
    for name, needed in assigned.items():
        placed = player.ceremony_stones_on_tile.get(name, 0)
        in_inventory = player.inventory.get(name, 0)
        gap = needed - placed - in_inventory
        if gap > 0:
            missing[name] = gap
    return missing


def role_stones_placed_on_rally(
    player: PlayerState,
    map_model: MapModel,
    next_level: int,
) -> bool:
    if player.rally_position is None:
        return False
    assigned = stones_for_role(player.role, next_level)
    for name, needed in assigned.items():
        if player.ceremony_stones_on_tile.get(name, 0) < needed:
            return False
    return bool(assigned)
