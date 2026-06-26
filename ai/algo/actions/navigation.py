"""Movement: pathfinding and navigation toward a target or broadcast."""

from collections import deque

from algo.world.map_model import (
    Direction,
    MapModel,
    Orientation,
    Position,
    VISION_OFFSETS,
    rotate_offset,
)
from algo.world.player_state import PlayerState, LeaderInfo, Role

# Number of Forward steps to queue per broadcast direction.
_NAV_STEPS = 1


def compute_path_commands(
    map_model: MapModel,
    start: Position,
    orientation: Orientation,
    target: Position,
) -> list[str]:
    if start == target:
        return []

    queue: deque[tuple[Position, list[Position]]] = deque([(start, [start])])
    seen = {start}

    while queue:
        current, path = queue.popleft()
        if current == target:
            return _path_to_commands(path, orientation, map_model)
        for neighbor in map_model.neighbors(current):
            if neighbor in seen:
                continue
            seen.add(neighbor)
            queue.append((neighbor, path + [neighbor]))
    return []


def _path_to_commands(
    path: list[Position],
    orientation: Orientation,
    map_model: MapModel,
) -> list[str]:
    commands: list[str] = []
    current_orientation = orientation
    current_pos = path[0]
    for next_pos in path[1:]:
        desired = _orientation_toward(current_pos, next_pos, map_model)
        while current_orientation != desired:
            turn = _shortest_turn(current_orientation, desired)
            if turn == Direction.LEFT:
                commands.append("Left")
                current_orientation = map_model.turn(
                    current_orientation, Direction.LEFT
                )
            else:
                commands.append("Right")
                current_orientation = map_model.turn(
                    current_orientation, Direction.RIGHT
                )
        commands.append("Forward")
        current_pos = next_pos
    return commands


def _orientation_toward(
    start: Position,
    target: Position,
    map_model: MapModel,
) -> Orientation:
    width, height = map_model.width, map_model.height
    dx = (target.x - start.x) % width
    dy = (target.y - start.y) % height
    if dx > width // 2:
        dx -= width
    if dy > height // 2:
        dy -= height
    if abs(dx) >= abs(dy):
        return Orientation.EAST if dx > 0 else Orientation.WEST
    return Orientation.SOUTH if dy > 0 else Orientation.NORTH


def _shortest_turn(current: Orientation, target: Orientation) -> Direction:
    delta = (int(target) - int(current)) % 4
    return Direction.LEFT if delta == 3 else Direction.RIGHT


def enqueue_path_to_target(
    player: PlayerState,
    map_model: MapModel,
    target: Position,
) -> str | None:
    if player.position == target:
        player.pending_moves.clear()
        return None
    commands = compute_path_commands(
        map_model,
        player.position,
        player.orientation,
        target,
    )
    if not commands:
        return None
    player.pending_moves = commands[1:]
    return commands[0]


def navigate_toward_vision_index(
    player: PlayerState,
    vision_index: int,
    map_model: MapModel,
) -> str | None:
    if vision_index <= 0 or vision_index >= len(VISION_OFFSETS):
        return None
    dx, dy = VISION_OFFSETS[vision_index]
    rdx, rdy = rotate_offset(dx, dy, player.orientation)
    target = Position(
        (player.position.x + rdx) % map_model.width,
        (player.position.y + rdy) % map_model.height,
    )
    return enqueue_path_to_target(player, map_model, target) or "Forward"


def is_on_leader_tile(player: PlayerState) -> bool:
    if player.role == Role.LEADER:
        return True
    if player.leader_info is None:
        return False
    return player.leader_info.direction == 0


def navigate_toward_leader_position(
    player: PlayerState,
    map_model: MapModel,
) -> str | None:
    if player.leader_info is None:
        return None

    k = player.leader_info.direction
    if k == 0:
        return None
    if k == -1:
        return "Look"

    # Build a movement sequence: first turn to face the leader, then walk
    # several steps in that direction.  On a toroidal 42×42 map this
    # dramatically speeds up convergence compared to the old 1-step approach.
    commands: list[str] = []
    if k == 1:
        commands = ["Forward"] * _NAV_STEPS
    elif k == 2:
        commands = ["Forward", "Left"] + ["Forward"] * _NAV_STEPS
    elif k == 3:
        commands = ["Left"] + ["Forward"] * _NAV_STEPS
    elif k == 4:
        commands = ["Left", "Left"] + ["Forward"] * _NAV_STEPS
    elif k == 5:
        commands = ["Left", "Left"] + ["Forward"] * _NAV_STEPS
    elif k == 6:
        commands = ["Right", "Right"] + ["Forward"] * _NAV_STEPS
    elif k == 7:
        commands = ["Right"] + ["Forward"] * _NAV_STEPS
    elif k == 8:
        commands = ["Forward", "Right"] + ["Forward"] * _NAV_STEPS

    player.leader_info = LeaderInfo(
        -1, player.leader_info.level, player.leader_info.uuid
    )

    if commands:
        player.pending_moves.extend(commands[1:])
        return commands[0]
    return "Look"
