from collections import deque
from dataclasses import dataclass, field
from enum import IntEnum

from algo.world.look_parser import TileView


class Orientation(IntEnum):
    NORTH = 0
    EAST = 1
    SOUTH = 2
    WEST = 3


class Direction(IntEnum):
    FORWARD = 0
    RIGHT = 1
    LEFT = 2


@dataclass(frozen=True)
class Position:
    x: int
    y: int


def wrap(value: int, size: int) -> int:
    return value % size


def build_vision_offsets() -> list[tuple[int, int]]:
    offsets: list[tuple[int, int]] = [(0, 0)]
    for depth in range(1, 8):
        for dx in range(-depth, depth + 1):
            offsets.append((dx, -depth))
    return offsets


VISION_OFFSETS = build_vision_offsets()


def rotate_offset(dx: int, dy: int, orientation: Orientation) -> tuple[int, int]:
    x, y = dx, dy
    for _ in range(int(orientation)):
        x, y = -y, x
    return x, y


@dataclass
class MapModel:
    width: int
    height: int
    tiles: dict[Position, list[str]] = field(default_factory=dict)
    visited: set[Position] = field(default_factory=set)
    food_tiles: set[Position] = field(default_factory=set)

    def apply_look(
        self,
        position: Position,
        orientation: Orientation,
        tiles: list[TileView],
    ) -> None:
        for tile in tiles:
            if tile.index >= len(VISION_OFFSETS):
                continue
            dx, dy = VISION_OFFSETS[tile.index]
            rdx, rdy = rotate_offset(dx, dy, orientation)
            pos = Position(
                wrap(position.x + rdx, self.width),
                wrap(position.y + rdy, self.height),
            )
            self.tiles[pos] = list(tile.objects)
            self.visited.add(pos)
            if "food" in tile.objects:
                self.food_tiles.add(pos)
            elif pos in self.food_tiles and "food" not in tile.objects:
                self.food_tiles.discard(pos)

    def add_resource(self, position: Position, resource: str) -> None:
        objects = list(self.tiles.get(position, []))
        objects.append(resource)
        self.tiles[position] = objects
        if resource == "food":
            self.food_tiles.add(position)

    def remove_resource(self, position: Position, resource: str) -> None:
        objects = list(self.tiles.get(position, []))
        if resource in objects:
            objects.remove(resource)
        self.tiles[position] = objects
        if resource == "food" and resource not in objects:
            self.food_tiles.discard(position)

    def move(self, position: Position, orientation: Orientation) -> Position:
        if orientation == Orientation.NORTH:
            return Position(position.x, wrap(position.y - 1, self.height))
        if orientation == Orientation.EAST:
            return Position(wrap(position.x + 1, self.width), position.y)
        if orientation == Orientation.SOUTH:
            return Position(position.x, wrap(position.y + 1, self.height))
        return Position(wrap(position.x - 1, self.width), position.y)

    def turn(self, orientation: Orientation, direction: Direction) -> Orientation:
        if direction == Direction.RIGHT:
            return Orientation((int(orientation) + 1) % 4)
        return Orientation((int(orientation) - 1) % 4)

    def neighbors(self, position: Position) -> list[Position]:
        return [
            Position(wrap(position.x, self.width), wrap(position.y - 1, self.height)),
            Position(wrap(position.x + 1, self.width), wrap(position.y, self.height)),
            Position(wrap(position.x, self.width), wrap(position.y + 1, self.height)),
            Position(wrap(position.x - 1, self.width), wrap(position.y, self.height)),
        ]

    def find_nearest_tile(self, start: Position, predicate) -> Position | None:
        queue: deque[Position] = deque([start])
        seen = {start}
        while queue:
            current = queue.popleft()
            if predicate(current, self.tiles.get(current, [])):
                return current
            for neighbor in self.neighbors(current):
                if neighbor in seen:
                    continue
                if neighbor not in self.tiles:
                    continue
                seen.add(neighbor)
                queue.append(neighbor)
        return None

    def find_nearest_frontier(self, start: Position) -> Position | None:
        frontier = self.frontier()
        if not frontier:
            return None
        return min(
            frontier,
            key=lambda pos: abs(pos.x - start.x) + abs(pos.y - start.y),
        )

    def frontier(self) -> list[Position]:
        unknown: list[Position] = []
        for pos in self.visited:
            for neighbor in self.neighbors(pos):
                if neighbor not in self.visited:
                    unknown.append(neighbor)
        return unknown
