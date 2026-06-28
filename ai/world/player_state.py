from dataclasses import dataclass, field
from enum import Enum, auto

from world.look_parser import TileView
from world.map_model import Orientation, Position

FOOD_LIFETIME = 126
FOOD_URGENT_THRESHOLD = 8
DEFAULT_TIMEOUT_SEC = 900


class GamePhase(Enum):
    FORAGE_EMERGENCY = auto()
    SOLO_L1_L2 = auto()
    RENDEZVOUS = auto()
    DUO_PREPARE = auto()
    DUO_INCANT = auto()


class Role(Enum):
    LEADER = "leader"
    FOLLOWER = "follower"


@dataclass(frozen=True)
class LeaderInfo:
    position: Position
    level: int
    direction: int = 0


@dataclass
class TeamIntel:
    partner_haves: dict[str, int] = field(default_factory=dict)
    partner_needs: dict[int, dict[str, int]] = field(default_factory=dict)
    hints: dict[Position, set[str]] = field(default_factory=dict)
    leader_waiting: bool = False


ELEVATION_REQUIREMENTS: dict[int, tuple[int, dict[str, int]]] = {
    2: (1, {"linemate": 1}),
    3: (2, {"linemate": 1, "deraumere": 1, "sibur": 1}),
    4: (2, {"linemate": 2, "sibur": 1, "phiras": 2}),
    5: (4, {"linemate": 1, "deraumere": 1, "sibur": 2, "phiras": 1}),
    6: (4, {"linemate": 1, "deraumere": 2, "sibur": 1, "mendiane": 3}),
    7: (6, {"linemate": 1, "deraumere": 2, "sibur": 3, "phiras": 1}),
    8: (6, {"linemate": 2, "deraumere": 2, "sibur": 2, "mendiane": 1, "phiras": 1}),
}


@dataclass
class PlayerState:
    level: int = 1
    inventory: dict[str, int] = field(default_factory=dict)
    spawn_position: Position = field(default_factory=lambda: Position(0, 0))
    position: Position = field(default_factory=lambda: Position(0, 0))
    orientation: Orientation = Orientation.NORTH
    available_slots: int = 0
    alive: bool = True
    waiting_partner: bool = False
    partner_ready: bool = False
    actions_since_inventory: int = 0
    role: Role = Role.LEADER
    leader_info: LeaderInfo | None = None
    ready_level: int | None = None
    pending_moves: list[str] = field(default_factory=list)
    broadcast_cooldown: int = 0
    timeout_sec: int = DEFAULT_TIMEOUT_SEC
    last_look_tiles: list[TileView] = field(default_factory=list)
    rally_position: Position | None = None
    phase: GamePhase = GamePhase.SOLO_L1_L2
    team_intel: TeamIntel = field(default_factory=TeamIntel)
    pending_broadcast: str | None = None

    def food_count(self) -> int:
        return self.inventory.get("food", 0)

    def food_urgency(self) -> bool:
        return self.food_count() <= FOOD_URGENT_THRESHOLD

    def missing_for_level(self, target_level: int) -> dict[str, int]:
        if target_level not in ELEVATION_REQUIREMENTS:
            return {}
        _, resources = ELEVATION_REQUIREMENTS[target_level]
        missing: dict[str, int] = {}
        for name, needed in resources.items():
            have = self.inventory.get(name, 0)
            if have < needed:
                missing[name] = needed - have
        return missing

    def players_required(self, target_level: int) -> int:
        if target_level not in ELEVATION_REQUIREMENTS:
            return 1
        players, _ = ELEVATION_REQUIREMENTS[target_level]
        return players

    def update_inventory(self, inventory: dict[str, int]) -> None:
        self.inventory = dict(inventory)
        self.actions_since_inventory = 0
        if self.food_count() <= 0:
            self.alive = False

    def count_on_tile(self, tile_objects: list[str], name: str) -> int:
        return sum(1 for obj in tile_objects if obj == name)

    def count_players_on_tile(self, tile_objects: list[str]) -> int:
        others = sum(1 for obj in tile_objects if obj == "player")
        return max(1, others)

    def pop_pending_move(self) -> str | None:
        if not self.pending_moves:
            return None
        return self.pending_moves.pop(0)
