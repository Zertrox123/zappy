import socket
import uuid as _uuid
from typing import Callable, Optional

from algo.world.broadcast_cipher import decrypt_broadcast, encrypt_broadcast


def parse_level(line: str) -> Optional[int]:
    prefix = "Current level:"
    if not line.startswith(prefix):
        return None
    rest = line[len(prefix) :].strip()
    try:
        return int(rest)
    except ValueError:
        return None


def parse_broadcast(line: str) -> Optional[tuple[int, str]]:
    if not line.startswith("message "):
        return None
    rest = line[len("message ") :]
    comma = rest.find(",")
    if comma < 0:
        return None
    direction_s = rest[:comma].strip()
    text = rest[comma + 1 :].strip()
    try:
        direction = int(direction_s)
    except ValueError:
        return None
    return direction, text


def format_status(level: int, *, alive: bool = True) -> str:
    return f"level={level} alive={'True' if alive else 'False'}"


def emit_status(level: int, *, alive: bool = True) -> None:
    print(format_status(level, alive=alive), flush=True)


class ZappyClient:
    def __init__(
        self,
        hostname: str,
        port: int,
        team_name: str,
        on_level: Optional[Callable[[int], None]] = None,
        on_broadcast: Optional[Callable[[int, str], None]] = None,
    ):
        self.hostname = hostname
        self.port = port
        self.team_name = team_name
        self.on_level = on_level
        self.on_broadcast = on_broadcast
        self.sock: Optional[socket.socket] = None
        self.buffer = ""
        self.level = 1
        self.slots = 0
        self.pending_broadcasts: list[tuple[int, str]] = []
        self.last_broadcast_dir: Optional[int] = None
        self.last_broadcast_text: Optional[str] = None
        self.incantation_underway: bool = False
        self.sender_id = _uuid.uuid4().hex[:6]
        self.broadcast_seq = 0
        self.peer_last_seq: dict[str, int] = {}

    def connect(self) -> None:
        if self.sock:
            self.sock.close()
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.connect((self.hostname, self.port))
        self.buffer = ""

    def close(self) -> None:
        if self.sock:
            self.sock.close()
            self.sock = None

    def send(self, message: str) -> None:
        if not self.sock:
            raise OSError("Not connected")
        self.sock.sendall(f"{message}\n".encode())

    def receive_raw(self) -> str:
        if not self.sock:
            raise OSError("Not connected")
        while "\n" not in self.buffer:
            data = self.sock.recv(4096).decode()
            if not data:
                raise ConnectionError("Server closed the connection.")
            self.buffer += data
        line, self.buffer = self.buffer.split("\n", 1)
        return line.strip()

    def _decode_broadcast(self, text: str) -> Optional[str]:
        """Return the plaintext of an authentic team message, else None.

        Anything that does not decrypt under our key with a valid tag is enemy
        traffic (plaintext spam, foreign ciphers, forgeries) and is dropped.
        Replayed copies of our own messages are dropped via the per-sender
        sequence number.
        """
        decoded = decrypt_broadcast(text, self.team_name)
        if decoded is None:
            return None
        sender, seq, plaintext = decoded
        if seq <= self.peer_last_seq.get(sender, -1):
            return None
        self.peer_last_seq[sender] = seq
        return plaintext

    def _store_broadcast(self, direction: int, text: str) -> None:
        plaintext = self._decode_broadcast(text)
        if plaintext is None:
            return
        self.last_broadcast_dir = direction
        self.last_broadcast_text = plaintext
        self.pending_broadcasts.append((direction, plaintext))
        if self.on_broadcast:
            self.on_broadcast(direction, plaintext)

    def handle_async_line(self, line: str) -> bool:
        if line.startswith("Current level:"):
            level = parse_level(line)
            if level is not None:
                self.level = level
                if self.on_level:
                    self.on_level(level)
            self.incantation_underway = False
            return True
        if line.startswith("Elevation"):
            self.incantation_underway = True
            print(line)
            return True
        broadcast = parse_broadcast(line)
        if broadcast is not None:
            direction, text = broadcast
            self._store_broadcast(direction, text)
            return True
        if line.startswith("eject"):
            return True
        return False

    def drain_broadcasts(self) -> list[tuple[int, str]]:
        pending = list(self.pending_broadcasts)
        self.pending_broadcasts.clear()
        return pending

    def receive_response(self) -> str:
        while True:
            line = self.receive_raw()
            if self.handle_async_line(line):
                continue
            if line == "ko" and getattr(self, "incantation_underway", False):
                self.incantation_underway = False
                continue
            return line

    def receive_incantation_response(self) -> str:
        while True:
            line = self.receive_raw()
            if line.startswith("Current level:"):
                self.handle_async_line(line)
                return "ok"
            if line in ("ko", "dead"):
                if line == "ko":
                    self.incantation_underway = False
                return line
            if self.handle_async_line(line):
                continue
            return line

    def handshake(self) -> None:
        welcome = self.receive_raw()
        if welcome != "WELCOME":
            raise ValueError("No WELCOME received")
        self.send(self.team_name)
        slots = self.receive_raw()
        if slots == "ko":
            raise ValueError("Invalid team or no slots left.")
        try:
            self.slots = int(slots)
        except ValueError:
            self.slots = 0
        dims = self.receive_raw()
        try:
            parts = dims.split()
            self.map_width = int(parts[0])
            self.map_height = int(parts[1])
        except (ValueError, IndexError):
            self.map_width = 42
            self.map_height = 42

    def query_inventory(self) -> str:
        self.send("Inventory")
        return self.receive_response()

    def query_look(self) -> str:
        self.send("Look")
        return self.receive_response()

    def query_connect_nbr(self) -> str:
        self.send("Connect_nbr")
        return self.receive_response()

    def send_action(self, action: str) -> str:
        self.send(action)
        if action == "Incantation":
            return self.receive_incantation_response()
        return self.receive_response()

    def send_broadcast(self, text: str) -> str:
        self.broadcast_seq += 1
        encrypted = encrypt_broadcast(
            text, self.team_name, self.sender_id, self.broadcast_seq
        )
        self.send(f"Broadcast {encrypted}")
        return self.receive_response()

    def send_raw_broadcast(self, text: str) -> str:
        """Send an UNencrypted broadcast: decoy chatter aimed at rival teams.

        Our own bots reject it (it does not carry our cipher tag), but rival AIs
        that parse loose plaintext commands may chase it.
        """
        self.send(f"Broadcast {text}")
        return self.receive_response()
