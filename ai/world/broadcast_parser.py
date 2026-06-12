from dataclasses import dataclass
import re


@dataclass(frozen=True)
class Broadcast:
    message: str
    direction: int


_POS_RE = re.compile(r"^POS (\d+) (\d+) L(\d+)$")
_READY_RE = re.compile(r"^READY L(\d+)$")
_NEED_RE = re.compile(r"^NEED L(\d+) (\w+)$")
_HAVE_RE = re.compile(r"^HAVE (\w+)(?: (\d+))?$")
_HINT_RE = re.compile(r"^HINT (\d+) (\d+) (\w+)$")
_WAIT_RE = re.compile(r"^WAIT$")


def parse_broadcast(line: str) -> Broadcast | None:
    text = line.strip()
    parts = text.rsplit(",", 1)
    if len(parts) != 2:
        return None
    message, direction_s = parts[0].strip(), parts[1].strip()
    if not direction_s.isdigit():
        return None
    return Broadcast(message=message, direction=int(direction_s))


def parse_pos_message(message: str) -> tuple[int, int, int] | None:
    match = _POS_RE.match(message.strip())
    if not match:
        return None
    return int(match.group(1)), int(match.group(2)), int(match.group(3))


def parse_ready_message(message: str) -> int | None:
    match = _READY_RE.match(message.strip())
    if not match:
        return None
    return int(match.group(1))


def parse_need_message(message: str) -> tuple[int, str] | None:
    match = _NEED_RE.match(message.strip())
    if not match:
        return None
    return int(match.group(1)), match.group(2)


def parse_have_message(message: str) -> tuple[str, int] | None:
    match = _HAVE_RE.match(message.strip())
    if not match:
        return None
    count = int(match.group(2)) if match.group(2) else 1
    return match.group(1), count


def parse_hint_message(message: str) -> tuple[int, int, str] | None:
    match = _HINT_RE.match(message.strip())
    if not match:
        return None
    return int(match.group(1)), int(match.group(2)), match.group(3)


def parse_wait_message(message: str) -> bool:
    return _WAIT_RE.match(message.strip()) is not None
