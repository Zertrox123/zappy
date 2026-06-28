import socket
import threading
from dataclasses import dataclass, field


@dataclass
class WorldState:
    width: int = 5
    height: int = 5
    level: int = 1
    inventory: dict[str, int] = field(default_factory=lambda: {"food": 10})
    tiles: dict[tuple[int, int], set[str]] = field(default_factory=dict)
    partner_on_tile: bool = False

    def look_payload(self) -> str:
        tile_objects = sorted(self.tiles.get((0, 0), set()))
        objects = ["player", *tile_objects]
        if self.partner_on_tile:
            objects.append("player")
        return f"[{' '.join(objects)}]"


class StatefulMockServer:
    def __init__(
        self, world: WorldState | None = None, team_name: str = "team"
    ) -> None:
        self.world = world or WorldState()
        self.team_name = team_name
        self.port = 0
        self._thread: threading.Thread | None = None
        self._stop = threading.Event()
        self.commands: list[str] = []

    def start(self) -> int:
        listener = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        listener.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        listener.bind(("127.0.0.1", 0))
        listener.listen(1)
        self.port = listener.getsockname()[1]
        self._thread = threading.Thread(
            target=self._serve, args=(listener,), daemon=True
        )
        self._thread.start()
        return self.port

    def stop(self) -> None:
        self._stop.set()
        if self._thread is not None:
            self._thread.join(timeout=2)

    def _serve(self, listener: socket.socket) -> None:
        try:
            conn, _addr = listener.accept()
            conn.sendall(b"WELCOME\n")
            line = self._readline(conn)
            if line != self.team_name:
                return
            conn.sendall(b"4\n")
            conn.sendall(f"{self.world.width} {self.world.height}\n".encode())
            while not self._stop.is_set():
                command = self._readline(conn)
                if command is None:
                    break
                self.commands.append(command)
                self._handle(conn, command)
        finally:
            listener.close()

    def _readline(self, conn: socket.socket) -> str | None:
        buffer = b""
        while b"\n" not in buffer:
            try:
                chunk = conn.recv(4096)
            except OSError:
                return None
            if not chunk:
                return None
            buffer += chunk
        return buffer.split(b"\n", 1)[0].decode().strip()

    def _handle(self, conn: socket.socket, command: str) -> None:
        if command == "Look":
            conn.sendall(b"ok\n")
            conn.sendall(f"{self.world.look_payload()}\n".encode())
            return
        if command == "Inventory":
            parts = [f"{name} {count}" for name, count in self.world.inventory.items()]
            conn.sendall(b"ok\n")
            conn.sendall(f"[{', '.join(parts)}]\n".encode())
            return
        if command == "Connect_nbr":
            conn.sendall(b"4\n")
            return
        if command.startswith("Take "):
            resource = command.split(" ", 1)[1]
            tile = self.world.tiles.setdefault((0, 0), set())
            if resource in tile:
                tile.remove(resource)
                self.world.inventory[resource] = (
                    self.world.inventory.get(resource, 0) + 1
                )
                conn.sendall(b"ok\n")
            else:
                conn.sendall(b"ko\n")
            return
        if command.startswith("Set "):
            resource = command.split(" ", 1)[1]
            if self.world.inventory.get(resource, 0) > 0:
                self.world.inventory[resource] -= 1
                self.world.tiles.setdefault((0, 0), set()).add(resource)
                conn.sendall(b"ok\n")
            else:
                conn.sendall(b"ko\n")
            return
        if command == "Incantation":
            required = _requirements_for(self.world.level + 1)
            if required and self._can_incant(required):
                self.world.level += 1
                conn.sendall(b"ok\n")
            else:
                conn.sendall(b"ko\n")
            return
        if command in {
            "Forward",
            "Right",
            "Left",
            "Fork",
            "Broadcast NEED_PARTNER",
        } or command.startswith("Broadcast "):
            conn.sendall(b"ok\n")
            return
        conn.sendall(b"ko\n")

    def _can_incant(self, required: tuple[int, dict[str, int]]) -> bool:
        players, resources = required
        tile = self.world.tiles.get((0, 0), set())
        player_count = 1 + (1 if self.world.partner_on_tile else 0)
        if player_count < players:
            return False
        for name, needed in resources.items():
            have = self.world.inventory.get(name, 0) + sum(
                1 for obj in tile if obj == name
            )
            if have < needed:
                return False
        return True


def _requirements_for(level: int) -> tuple[int, dict[str, int]] | None:
    table = {
        2: (1, {"linemate": 1}),
        3: (2, {"linemate": 1, "deraumere": 1, "sibur": 1}),
        4: (2, {"linemate": 2, "sibur": 1, "phiras": 2}),
    }
    return table.get(level)
