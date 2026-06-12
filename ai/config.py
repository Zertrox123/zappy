from dataclasses import dataclass

EXIT_USAGE = 84

USAGE = """\
USAGE: ./zappy_ai -p port -n name -h machine

option\t\tdescription
-p port\t\tport number
-n name\t\tname of the team
-h machine\tname of the machine; localhost by default
"""

DEFAULT_HOST = "localhost"


@dataclass(frozen=True)
class AiConfig:
    port: int
    team_name: str
    hostname: str = DEFAULT_HOST


class ConfigParseError(Exception):
    pass


def parse_args(args: list[str]) -> AiConfig:
    port: int | None = None
    team_name: str | None = None
    hostname: str | None = None

    index = 1
    while index < len(args):
        flag = args[index]
        if flag == "-p":
            index += 1
            port = _parse_port(_require(args, index, "-p"))
        elif flag == "-n":
            index += 1
            team_name = _parse_team_name(_require(args, index, "-n"))
        elif flag == "-h":
            index += 1
            hostname = _parse_hostname(_require(args, index, "-h"))
        else:
            raise ConfigParseError(f"unknown argument: {flag}")
        index += 1

    if port is None:
        raise ConfigParseError("missing required argument: -p")
    if team_name is None:
        raise ConfigParseError("missing required argument: -n")
    return AiConfig(port=port, team_name=team_name, hostname=hostname or DEFAULT_HOST)


def _require(args: list[str], index: int, flag: str) -> str:
    if index >= len(args):
        raise ConfigParseError(f"missing value for {flag}")
    return args[index]


def _parse_port(value: str) -> int:
    try:
        port = int(value)
    except ValueError as exc:
        raise ConfigParseError(f"invalid value for -p: {value}") from exc
    if port <= 0 or port > 65535:
        raise ConfigParseError(f"invalid value for -p: {value}")
    return port


def _parse_team_name(value: str) -> str:
    if not value:
        raise ConfigParseError("team name cannot be empty")
    return value


def _parse_hostname(value: str) -> str:
    if not value:
        raise ConfigParseError(f"invalid value for -h: {value}")
    return value
