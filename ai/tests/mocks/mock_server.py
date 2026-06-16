import socket
import threading
from dataclasses import dataclass, field
from typing import Callable

ScriptStep = tuple[str, str | Callable[[str], bool] | None]


@dataclass
class MockServer:
    host: str = "127.0.0.1"
    port: int = 0
    team_name: str = "team"
    script: list[ScriptStep] = field(default_factory=list)
    _thread: threading.Thread | None = field(default=None, init=False, repr=False)
    _sock: socket.socket | None = field(default=None, init=False, repr=False)
    _received: list[str] = field(default_factory=list, init=False, repr=False)

    def start(self) -> int:
        listener = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        listener.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        listener.bind((self.host, 0))
        listener.listen(1)
        self.port = listener.getsockname()[1]
        self._thread = threading.Thread(
            target=self._serve, args=(listener,), daemon=True
        )
        self._thread.start()
        return self.port

    def stop(self) -> None:
        if self._sock is not None:
            try:
                self._sock.close()
            except OSError:
                pass
        if self._thread is not None:
            self._thread.join(timeout=2)

    @property
    def received(self) -> list[str]:
        return list(self._received)

    def _serve(self, listener: socket.socket) -> None:
        try:
            conn, _addr = listener.accept()
            self._sock = conn
            buffer = b""
            for kind, value in self.script:
                if kind == "send":
                    conn.sendall(f"{value}\n".encode())
                elif kind == "recv":
                    while b"\n" not in buffer:
                        chunk = conn.recv(4096)
                        if not chunk:
                            return
                        buffer += chunk
                    line, buffer = buffer.split(b"\n", 1)
                    text = line.decode().strip()
                    self._received.append(text)
                    if callable(value):
                        if not value(text):
                            return
                    elif value is not None and text != value:
                        return
        finally:
            listener.close()
