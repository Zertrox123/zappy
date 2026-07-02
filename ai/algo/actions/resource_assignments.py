"""Which stones each role must provide for a given target level."""

from algo.world.player_state import Role

_FULL_6 = {"linemate": 1, "deraumere": 2, "sibur": 1, "mendiane": 3}
_FULL_7 = {"linemate": 1, "deraumere": 2, "sibur": 3, "phiras": 1}
_FULL_8 = {
    "linemate": 2,
    "deraumere": 2,
    "sibur": 2,
    "mendiane": 2,
    "phiras": 2,
    "thystame": 1,
}

_LEADER_STONES: dict[int, dict[str, int]] = {
    3: {"linemate": 1, "deraumere": 1, "sibur": 1},
    4: {"linemate": 2, "sibur": 1, "phiras": 2},
    5: {"linemate": 1, "deraumere": 1, "sibur": 2, "phiras": 1},
    6: {},
    7: {},
    8: {},
}

_FOLLOWER_STONES: dict[int, dict[str, int]] = {
    3: {"linemate": 1},
    4: {"linemate": 1, "phiras": 1},
    5: {"sibur": 1},
    6: dict(_FULL_6),
    7: dict(_FULL_7),
    8: dict(_FULL_8),
}


def stones_for_role(role: Role, target_level: int) -> dict[str, int]:
    if role == Role.LEADER:
        return dict(_LEADER_STONES.get(target_level, {}))
    return dict(_FOLLOWER_STONES.get(target_level, {}))


def missing_stones_in_inventory(
    role: Role,
    target_level: int,
    inventory: dict[str, int],
) -> dict[str, int]:
    assigned = stones_for_role(role, target_level)
    missing: dict[str, int] = {}
    for name, needed in assigned.items():
        have = inventory.get(name, 0)
        if have < needed:
            missing[name] = needed - have
    return missing
