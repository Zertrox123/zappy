"""High-level decision logic for team play (Level 2+)."""

from algo.actions.exploration import decide_exploration_action
from algo.actions.incantation import can_start_incantation
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

# ── Food thresholds ─────────────────────────────────────────────────────
# Thresholds for when to prioritize food gathering vs team tasks
FOOD_CRITICAL_THRESHOLD = 5
FOOD_LOW_THRESHOLD = 12
FOOD_HIGH_THRESHOLD = 20
BROADCAST_INTERVAL = 6


# ── Helpers ──────────────────────────────────────────────────────────────


def _stones_complete_on_tile(player: PlayerState, next_level: int) -> bool:
    """Check whether ALL stones required for *next_level* are on the tile.

    Checks both Look data and our own placement tracking.
    """
    if next_level not in ELEVATION_REQUIREMENTS:
        return False
    _, resources = ELEVATION_REQUIREMENTS[next_level]

    tile_objects: list[str] = []
    for look_tile in player.last_look_tiles:
        if look_tile.index == 0:
            tile_objects = list(look_tile.objects)
            break

    for name, needed in resources.items():
        visible = sum(1 for obj in tile_objects if obj == name)
        placed = player.ceremony_stones_on_tile.get(name, 0)
        if max(visible, placed) < needed:
            return False
    return True


def _has_all_role_stones_in_inventory(player: PlayerState, next_level: int) -> bool:
    assigned = stones_for_role(player.role, next_level)
    return all(
        player.inventory.get(name, 0) >= needed for name, needed in assigned.items()
    )


def _needs_more_stones(player: PlayerState, next_level: int) -> bool:
    """Does this player still need to gather stones from the map?"""
    if next_level not in ELEVATION_REQUIREMENTS:
        return False

    assigned = stones_for_role(player.role, next_level)
    for resource, needed in assigned.items():
        have_inv = player.inventory.get(resource, 0)
        have_placed = player.ceremony_stones_on_tile.get(resource, 0)
        if have_inv + have_placed < needed:
            return True
    return False


# ── Food priority ────────────────────────────────────────────────────────


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

    # At the ceremony tile, only suppress food mode if we're comfortable
    # Otherwise we starve waiting for followers.
    if on_leader and food > FOOD_CRITICAL_THRESHOLD:
        player.is_gathering_food = False

    if player.role == Role.LEADER and food > FOOD_LOW_THRESHOLD + 2:
        player.is_gathering_food = False

    if not player.is_gathering_food:
        # Opportunistic eating: grab food if we happen to step on it
        current_tile = map_model.tiles.get(player.position, [])
        if food < FOOD_HIGH_THRESHOLD and "food" in current_tile:
            return "Take food"

        # Opportunistic stone gathering to deliver to rally point
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


# ── Leader ───────────────────────────────────────────────────────────────


def decide_leader_team_action(
    player: PlayerState, map_model: MapModel, target_level: int
) -> str:
    next_level = player.level + 1
    if player.rally_position is None:
        player.rally_position = player.position

    at_rally = player.position == player.rally_position

    # ── Phase 1: gather stones ──────────────────────────────────────────
    if _needs_more_stones(player, next_level):
        gathered = gather_assigned_stones(player, map_model, target_level)
        if gathered is not None:
            return gathered
        if not player.last_look_tiles:
            return "Look"
        return decide_exploration_action(player, map_model)

    # ── Phase 2: return to rally ────────────────────────────────────────
    if not at_rally:
        move = enqueue_path_to_target(player, map_model, player.rally_position)
        if move is not None:
            return move

    # ── Phase 3: place stones ───────────────────────────────────────────
    place_action = place_stones_from_inventory(
        player, map_model, next_level, player.rally_position
    )
    if place_action is not None:
        return place_action

    # ── Phase 4: attempt incantation ────────────────────────

    if can_start_incantation(player, map_model, next_level, target_level):
        return "Incantation"

    # Alternate between looking for arriving followers and broadcasting our position
    if player.pos_broadcast_sent:
        player.pos_broadcast_sent = False
        return "Look"
    else:
        player.pos_broadcast_sent = True
        return f"Broadcast {format_regroup_broadcast(next_level, player.uuid)}"


# ── Follower ─────────────────────────────────────────────────────────────


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

    # ── Phase 1: gather stones ──────────────────────────────────────────
    if _needs_more_stones(player, next_level):
        # If we happen to be at the leader tile, walk away first to explore.
        if is_on_leader_tile(player):
            return "Forward"

        gathered = gather_assigned_stones(player, map_model, target_level)
        if gathered is not None:
            return gathered

        if not player.last_look_tiles:
            return "Look"
        return decide_exploration_action(player, map_model)

    # ── Phase 2: navigate to leader ─────────────────────────────────────
    if not is_on_leader_tile(player):
        if player.leader_info is not None and player.leader_info.level >= player.level:
            move = navigate_toward_leader_position(player, map_model)
            if move is not None:
                return move
            # If we lost direction (-1) or finished our path, WAIT for next broadcast.
            # Do NOT wander off randomly!
            return "Look"
        return decide_exploration_action(player, map_model)

    # ── Phase 3: place any stones the tile still needs ──────────────────
    place_action = place_stones_from_inventory(
        player, map_model, next_level, player.position
    )
    if place_action is not None:
        return place_action

    # ── Phase 4: attempt incantation ────────────────────────────────────
    # Only the leader starts the incantation to avoid concurrent incantation commands.

    # Wait at the leader tile, keep refreshing vision.
    return "Look"


# ── Entry point ──────────────────────────────────────────────────────────


def decide_team_action(
    player: PlayerState, map_model: MapModel, target_level: int
) -> str:
    next_level = player.level + 1
    if next_level > target_level or next_level not in ELEVATION_REQUIREMENTS:
        return decide_exploration_action(player, map_model)

    # Periodic broadcast removed so leader doesn't drag followers around while searching for stones.
    # Leader will only broadcast when it has all stones and is waiting at the rally point.

    # 1. Survival takes absolute priority over grouping up or placing stones
    food_action = should_prioritize_food(player, map_model, next_level)
    if food_action is not None:
        return food_action

    # 2. Proceed with specific role logic
    if player.role == Role.LEADER:
        return decide_leader_team_action(player, map_model, target_level)
    return decide_follower_team_action(player, map_model, target_level)
