from dataclasses import dataclass, field
from enum import Enum
import uuid

from algo.world.elevation_rules import ELEVATION_REQUIREMENTS
from algo.world.look_parser import TileView
from algo.world.map_model import Orientation, Position


class Role(Enum):
    LEADER = "leader"
    FOLLOWER = "follower"


@dataclass(frozen=True)
class LeaderInfo:
    direction: int
    level: int
    uuid: str = ""


@dataclass
class PlayerState:
    level: int = 1
    inventory: dict[str, int] = field(default_factory=dict)
    position: Position = field(default_factory=lambda: Position(0, 0))
    orientation: Orientation = Orientation.NORTH
    role: Role = Role.LEADER
    leader_info: LeaderInfo | None = None
    pending_moves: list[str] = field(default_factory=list)
    last_look_tiles: list[TileView] = field(default_factory=list)
    ticks_since_leader: int = 0
    rally_position: Position | None = None
    ceremony_stones_on_tile: dict[str, int] = field(default_factory=dict)
    pos_broadcast_sent: bool = False
    is_gathering_food: bool = False
    uuid: str = field(default_factory=lambda: str(uuid.uuid4())[:8])
    inventory_timer: int = 0

    def food_count(self) -> int:
        return self.inventory.get("food", 0)

    def players_required(self, target_level: int) -> int:
        if target_level not in ELEVATION_REQUIREMENTS:
            return 1
        players, _ = ELEVATION_REQUIREMENTS[target_level]
        return players

    def update_inventory(self, inventory: dict[str, int]) -> None:
        self.inventory = dict(inventory)

    def count_on_tile(self, tile_objects: list[str], name: str) -> int:
        return sum(1 for obj in tile_objects if obj == name)

    def count_players_on_tile(self, tile_objects: list[str]) -> int:
        return sum(1 for obj in tile_objects if obj == "player")

    def pop_pending_move(self) -> str | None:
        if not self.pending_moves:
            return None
        return self.pending_moves.pop(0)
