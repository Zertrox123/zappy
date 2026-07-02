"""Fork when a ceremony needs more players than are on the rally tile."""

from algo.actions.incantation import can_start_incantation
from algo.world.elevation_rules import ELEVATION_REQUIREMENTS
from algo.world.map_model import MapModel
from algo.world.player_state import PlayerState, Role

FORK_FOOD_MIN = 22
FORK_COOLDOWN = 60
FORK_WAIT_TICKS = 300


def _players_on_current_tile(player: PlayerState) -> int:
    for look_tile in player.last_look_tiles:
        if look_tile.index == 0:
            return player.count_players_on_tile(look_tile.objects)
    return 1


def should_fork(
    player: PlayerState,
    next_level: int,
    connect_nbr: int,
) -> bool:
    if not player.is_primary_leader or player.role != Role.LEADER:
        return False
    if connect_nbr <= 0:
        return False
    if next_level not in ELEVATION_REQUIREMENTS:
        return False
    if next_level < 4:
        return False
    if player.food_count() < FORK_FOOD_MIN:
        return False
    if player.ticks_since_fork < FORK_COOLDOWN:
        return False
    if (
        player.ticks_since_fork < FORK_WAIT_TICKS
        and connect_nbr <= player.connect_nbr_at_fork
    ):
        return False

    required_players, _ = ELEVATION_REQUIREMENTS[next_level]
    if required_players <= 1:
        return False

    if player.rally_position is None or player.position != player.rally_position:
        return False

    return _players_on_current_tile(player) < required_players


def decide_fork_action(
    player: PlayerState,
    map_model: MapModel,
    target_level: int,
) -> str | None:
    next_level = player.level + 1
    if next_level > target_level:
        return None
    if can_start_incantation(player, map_model, next_level, target_level):
        return None
    if should_fork(player, next_level, player.connect_nbr):
        return "Fork"
    return None
