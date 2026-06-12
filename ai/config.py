from __future__ import annotations

from dataclasses import dataclass
from enum import Enum

EXIT_USAGE = 84

USAGE = """\
USAGE: ./zappy_ai -p port -n name -h machine

option\t\tdescription
-p port\t\tport number
-n name\t\tname of the team
-h machine\tname of the machine; localhost by default
"""

DEFAULT_HOST = "localhost"


class ConfigError(Enum):
    MISSING_VALUE = "missing_value"
    UNKNOWN_FLAG = "unknown_flag"
    MISSING_FLAG = "missing_flag"
    INVALID_VALUE = "invalid_value"
    EMPTY_TEAM_NAME = "empty_team_name"

    def format(self, *, flag: str = "", value: str = "") -> str:
        if self is ConfigError.MISSING_VALUE:
            return f"missing value for {flag}"
        if self is ConfigError.UNKNOWN_FLAG:
            return f"unknown argument: {flag}"
        if self is ConfigError.MISSING_FLAG:
            return f"missing required argument: {flag}"
        if self is ConfigError.INVALID_VALUE:
            return f"invalid value for {flag}: {value}"
        if self is ConfigError.EMPTY_TEAM_NAME:
            return "team name cannot be empty"
        raise AssertionError("unreachable")


@dataclass(frozen=True)
class AiConfig:
    port: int
    team_name: str
    hostname: str = DEFAULT_HOST


def parse_args(args: list[str]) -> AiConfig:
    port: int | None = None
    team_name: str | None = None
    hostname: str | None = None

    index = 1
    while index < len(args):
        flag = args[index]
        if flag == "-p":
            index += 1
            port = _parse_port(_next_value(args, index, "-p"))
            index += 1
        elif flag == "-n":
            index += 1
            team_name = _parse_team_name(_next_value(args, index, "-n"))
            index += 1
        elif flag == "-h":
            index += 1
            hostname = _parse_hostname(_next_value(args, index, "-h"))
            index += 1
        elif flag in ("--help",):
            raise HelpRequested()
        else:
            raise ConfigParseError(ConfigError.UNKNOWN_FLAG, flag=flag)
    if port is None:
        raise ConfigParseError(ConfigError.MISSING_FLAG, flag="-p")
    if team_name is None:
        raise ConfigParseError(ConfigError.MISSING_FLAG, flag="-n")
    return AiConfig(port=port, team_name=team_name, hostname=hostname or DEFAULT_HOST)


class HelpRequested(Exception):
    pass


class ConfigParseError(Exception):
    def __init__(self, kind: ConfigError, *, flag: str = "", value: str = "") -> None:
        self.kind = kind
        self.flag = flag
        self.value = value
        super().__init__(kind.format(flag=flag, value=value))


def _next_value(args: list[str], index: int, flag: str) -> str:
    if index >= len(args):
        raise ConfigParseError(ConfigError.MISSING_VALUE, flag=flag)
    return args[index]


def _parse_port(value: str) -> int:
    try:
        port = int(value)
    except ValueError as exc:
        raise ConfigParseError(ConfigError.INVALID_VALUE, flag="-p", value=value) from exc
    if port <= 0 or port > 65535:
        raise ConfigParseError(ConfigError.INVALID_VALUE, flag="-p", value=value)
    return port


def _parse_team_name(value: str) -> str:
    if not value:
        raise ConfigParseError(ConfigError.EMPTY_TEAM_NAME)
    return value


def _parse_hostname(value: str) -> str:
    if not value:
        raise ConfigParseError(ConfigError.INVALID_VALUE, flag="-h", value=value)
    return value
