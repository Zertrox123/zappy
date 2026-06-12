from dataclasses import dataclass, field
from enum import Enum

from client.command_queue import CommandQueue
from client.connection import Connection
from world.broadcast_parser import Broadcast, parse_broadcast


class ProtocolError(Exception):
    pass


class CommandStatus(Enum):
    OK = "ok"
    KO = "ko"


@dataclass(frozen=True)
class HandshakeInfo:
    available_slots: int
    width: int
    height: int


@dataclass(frozen=True)
class Response:
    status: CommandStatus
    payload: str | None = None


DIRECT_PAYLOAD_COMMANDS = frozenset({"Look", "Inventory"})
PAYLOAD_AFTER_OK_COMMANDS = frozenset({"Look", "Inventory"})


@dataclass
class ProtocolClient:
    conn: Connection
    queue: CommandQueue = field(default_factory=CommandQueue)
    broadcast_queue: list[Broadcast] = field(default_factory=list)
    pending_level_line: str | None = None

    def handshake(self, team_name: str) -> HandshakeInfo:
        welcome = self.conn.readline()
        if welcome != "WELCOME":
            raise ProtocolError(f"expected WELCOME, got {welcome!r}")
        self.conn.write_line(team_name)
        slots_line = self.conn.readline()
        map_line = self.conn.readline()
        if slots_line == "ko":
            raise ProtocolError("handshake rejected")
        try:
            slots = int(slots_line)
            width_s, height_s = map_line.split()
            return HandshakeInfo(slots, int(width_s), int(height_s))
        except ValueError as exc:
            raise ProtocolError("invalid handshake response") from exc

    def drain_broadcasts(self) -> list[Broadcast]:
        pending = list(self.broadcast_queue)
        self.broadcast_queue.clear()
        return pending

    def send_command(self, command: str) -> Response:
        self.queue.acquire()
        try:
            self.conn.write_line(command)
            for _ in range(32):
                line = self.conn.readline()
                response = _parse_response_line(command, line, self.conn, self)
                if response is not None:
                    return response
            raise ProtocolError("too many unsolicited server messages")
        finally:
            self.queue.complete()

    def forward(self) -> Response:
        return self.send_command("Forward")

    def right(self) -> Response:
        return self.send_command("Right")

    def left(self) -> Response:
        return self.send_command("Left")

    def look(self) -> Response:
        return self.send_command("Look")

    def inventory(self) -> Response:
        return self.send_command("Inventory")

    def take(self, resource: str) -> Response:
        return self.send_command(f"Take {resource}")

    def set_resource(self, resource: str) -> Response:
        return self.send_command(f"Set {resource}")

    def broadcast(self, message: str) -> Response:
        return self.send_command(f"Broadcast {message}")

    def connect_nbr(self) -> Response:
        return self.send_command("Connect_nbr")

    def fork(self) -> Response:
        return self.send_command("Fork")

    def incantation(self) -> Response:
        return self.send_command("Incantation")

    def eject(self) -> Response:
        return self.send_command("Eject")


def _command_name(command: str) -> str:
    return command.split(" ", 1)[0]


def _enqueue_unsolicited(line: str, client: ProtocolClient) -> bool:
    broadcast = parse_broadcast(line)
    if broadcast is not None:
        client.broadcast_queue.append(broadcast)
        return True
    if line.startswith("Current level:"):
        client.pending_level_line = line
        return True
    if line == "Elevation underway":
        return True
    if not line:
        return True
    return False


def _parse_response_line(
    command: str,
    line: str,
    conn: Connection,
    client: ProtocolClient,
) -> Response | None:
    cmd = _command_name(command)
    if line == "ko":
        return Response(CommandStatus.KO)
    if line == "dead":
        return Response(CommandStatus.KO, line)
    if line == "ok":
        if cmd in PAYLOAD_AFTER_OK_COMMANDS:
            payload = conn.readline()
            return Response(CommandStatus.OK, payload)
        return Response(CommandStatus.OK)
    if cmd in DIRECT_PAYLOAD_COMMANDS and line.startswith("["):
        return Response(CommandStatus.OK, line)
    if cmd == "Connect_nbr" and line.isdigit():
        return Response(CommandStatus.OK, line)
    if cmd == "Incantation" and line.startswith("Current level:"):
        return Response(CommandStatus.OK, line)
    if _enqueue_unsolicited(line, client):
        return None
    raise ProtocolError(f"unexpected response: {line!r}")
