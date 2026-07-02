"""High-level decision logic for team play (Level 2+)."""

import os

from algo.actions.deception import craft_decoy_message
from algo.actions.exploration import decide_exploration_action
from algo.actions.incantation import (
    can_start_incantation,
    stones_satisfied_on_tile,
)
from algo.actions.navigation import (
    enqueue_path_to_target,
    is_on_leader_tile,
    navigate_toward_leader_position,
)
from algo.actions.resource_assignments import stones_for_role
from algo.actions.resource_gathering import (
    gather_assigned_stones,
    place_stones_from_inventory,
)
from algo.actions.survival import decide_survival_action
from algo.world.broadcast_messages import format_regroup_broadcast
from algo.world.elevation_rules import ELEVATION_REQUIREMENTS
from algo.world.map_model import MapModel
from algo.world.player_state import PlayerState, Role

FOOD_CRITICAL_THRESHOLD = 5
FOOD_LOW_THRESHOLD = 12
FOOD_HIGH_THRESHOLD = 20
GROUP_WAIT_FROM_LEVEL = 3
DESIRED_GROUP_CAP = 6
RALLY_PATIENCE_TICKS = 90
CEREMONY_COMMIT_FOOD = 6
DECOY_ENABLED = os.environ.get("ZAPPY_DECOY", "1") != "0"
DECOY_EVERY_N_WAIT_TICKS = 6


def _players_on_current_tile(player: PlayerState) -> int:
    for look_tile in player.last_look_tiles:
        if look_tile.index == 0:
            return player.count_players_on_tile(look_tile.objects)
    return 1


def _desired_group_size(player: PlayerState, next_level: int, target_level: int) -> int:
    """How many players to hold for: the biggest group any remaining level needs."""
    if next_level < GROUP_WAIT_FROM_LEVEL:
        return player.players_required(next_level)
    sizes = [
        player.players_required(level)
        for level in range(next_level, target_level + 1)
        if level in ELEVATION_REQUIREMENTS
    ]
    if not sizes:
        return player.players_required(next_level)
    return min(DESIRED_GROUP_CAP, max(sizes))


def _leader_is_hosting(
    player: PlayerState, map_model: MapModel, next_level: int
) -> bool:
    """True when the leader is camped on the rally with every ceremony stone down.

    While hosting it holds position and eats only tile-local food rather than
    roaming off: a moving leader is a moving rally, and the followers (who home on
    its beacon) can never converge on a target that keeps walking away. The hunger
    bail in decide_leader_team_action incants before it can starve.
    """
    if player.rally_position is None or player.position != player.rally_position:
        return False
    if not stones_satisfied_on_tile(player, map_model, next_level):
        return False
    if player.food_count() <= FOOD_CRITICAL_THRESHOLD and _players_on_current_tile(
        player
    ) < player.players_required(next_level):
        return False
    return True


def _needs_more_stones(player: PlayerState, next_level: int) -> bool:
    if next_level not in ELEVATION_REQUIREMENTS:
        return False

    assigned = stones_for_role(player.role, next_level)
    for resource, needed in assigned.items():
        have_inv = player.inventory.get(resource, 0)
        have_placed = player.ceremony_stones_on_tile.get(resource, 0)
        if have_inv + have_placed < needed:
            return True
    return False


def should_prioritize_food(
    player: PlayerState, map_model: MapModel, next_level: int
) -> str | None:
    food = player.food_count()

    on_leader = (
        is_on_leader_tile(player)
        if player.role == Role.FOLLOWER
        else (
            player.rally_position is not None
            and player.position == player.rally_position
        )
    )

    if food <= FOOD_LOW_THRESHOLD:
        if not player.is_gathering_food:
            player.is_gathering_food = True
            player.pending_moves.clear()
    elif food >= FOOD_HIGH_THRESHOLD:
        player.is_gathering_food = False

    if on_leader and food > FOOD_CRITICAL_THRESHOLD:
        player.is_gathering_food = False

    if player.role == Role.LEADER and food > FOOD_LOW_THRESHOLD + 2:
        player.is_gathering_food = False

    if not player.is_gathering_food:
        current_tile = map_model.tiles.get(player.position, [])
        if food < FOOD_HIGH_THRESHOLD and "food" in current_tile:
            return "Take food"

        if not is_on_leader_tile(player):
            for obj in current_tile:
                if obj not in ["player", "food"]:
                    return f"Take {obj}"

        return None

    tile = map_model.tiles.get(player.position, [])
    if "food" in tile:
        return "Take food"

    survival_action = decide_survival_action(player, map_model)
    if survival_action is not None:
        return survival_action

    return decide_exploration_action(player, map_model)


def decide_leader_team_action(
    player: PlayerState, map_model: MapModel, target_level: int
) -> str:
    next_level = player.level + 1
    if player.rally_position is None:
        player.rally_position = player.position

    at_rally = player.position == player.rally_position

    if _needs_more_stones(player, next_level):
        player.rally_wait_ticks = 0
        gathered = gather_assigned_stones(player, map_model, target_level)
        if gathered is not None:
            return gathered
        if not player.last_look_tiles:
            return "Look"
        return decide_exploration_action(player, map_model)

    if not at_rally:
        player.rally_wait_ticks = 0
        move = enqueue_path_to_target(player, map_model, player.rally_position)
        if move is not None:
            return move

    place_action = place_stones_from_inventory(
        player, map_model, next_level, player.rally_position
    )
    if place_action is not None:
        return place_action

    if stones_satisfied_on_tile(player, map_model, next_level):
        player.rally_wait_ticks += 1
    else:
        player.rally_wait_ticks = 0

    if can_start_incantation(player, map_model, next_level, target_level):
        desired = _desired_group_size(player, next_level, target_level)
        present = _players_on_current_tile(player)
        if (
            present >= desired
            or player.rally_wait_ticks >= RALLY_PATIENCE_TICKS
            or player.food_count() < CEREMONY_COMMIT_FOOD
        ):
            player.rally_wait_ticks = 0
            return "Incantation"

    current_tile = map_model.tiles.get(player.position, [])
    if player.food_count() < FOOD_HIGH_THRESHOLD and "food" in current_tile:
        return "Take food"

    if player.pos_broadcast_sent:
        player.pos_broadcast_sent = False
        return "Look"
    player.pos_broadcast_sent = True
    return f"Broadcast {format_regroup_broadcast(next_level, player.uuid)}"


def decide_follower_team_action(
    player: PlayerState, map_model: MapModel, target_level: int
) -> str:
    next_level = player.level + 1

    player.ticks_since_leader += 1
    if player.ticks_since_leader > 100:
        player.role = Role.LEADER
        player.ticks_since_leader = 0
        player.rally_position = player.position
        return "Look"

    if _needs_more_stones(player, next_level):
        if is_on_leader_tile(player):
            return "Forward"

        gathered = gather_assigned_stones(player, map_model, target_level)
        if gathered is not None:
            return gathered

        if not player.last_look_tiles:
            return "Look"
        return decide_exploration_action(player, map_model)

    if not is_on_leader_tile(player):
        if player.leader_info is not None and player.leader_info.level >= player.level:
            move = navigate_toward_leader_position(player, map_model)
            if move is not None:
                return move
            return "Look"
        return decide_exploration_action(player, map_model)

    place_action = place_stones_from_inventory(
        player, map_model, next_level, player.position
    )
    if place_action is not None:
        return place_action

    if DECOY_ENABLED:
        player.decoy_tick += 1
        if player.decoy_tick % DECOY_EVERY_N_WAIT_TICKS == 0:
            return f"RawBroadcast {craft_decoy_message()}"

    return "Look"


def decide_team_action(
    player: PlayerState, map_model: MapModel, target_level: int
) -> str:
    next_level = player.level + 1
    if next_level > target_level or next_level not in ELEVATION_REQUIREMENTS:
        return decide_exploration_action(player, map_model)

    if player.role == Role.LEADER and _leader_is_hosting(player, map_model, next_level):
        return decide_leader_team_action(player, map_model, target_level)

    food_action = should_prioritize_food(player, map_model, next_level)
    if food_action is not None:
        return food_action

    if player.role == Role.LEADER:
        return decide_leader_team_action(player, map_model, target_level)
    return decide_follower_team_action(player, map_model, target_level)
