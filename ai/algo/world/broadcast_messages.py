"""Format and parse team broadcast payloads.

The reference server rejects messages containing spaces.
Use underscore-separated payloads (e.g. POS_0_0_L2).
"""

from dataclasses import dataclass
import re


@dataclass(frozen=True)
class TeamBroadcast:
    message: str
    direction: int


_REGROUP_PATTERN = re.compile(r"^REGROUP[_ ]L(\d+)(?:_(.+))?$")
_READY_PATTERN = re.compile(r"^READY[_ ]L(\d+)(?:_(.+))?$")


def format_regroup_broadcast(level: int, player_uuid: str = "") -> str:
    if player_uuid:
        return f"REGROUP_L{level}_{player_uuid}"
    return f"REGROUP_L{level}"


def format_ready_broadcast(level: int, player_uuid: str = "") -> str:
    if player_uuid:
        return f"READY_L{level}_{player_uuid}"
    return f"READY_L{level}"


def parse_regroup_message(message: str) -> tuple[int, str] | int | None:
    match = _REGROUP_PATTERN.match(message)
    if not match:
        return None
    level = int(match.group(1))
    if match.group(2):
        return level, match.group(2)
    return level


from typing import Union, Tuple


def parse_ready_message(message: str) -> Union[Tuple[int, str], int, None]:
    match = _READY_PATTERN.match(message.strip())
    if not match:
        return None
    level = int(match.group(1))
    uuid_str = match.group(2)
    if uuid_str:
        return level, uuid_str
    return level
