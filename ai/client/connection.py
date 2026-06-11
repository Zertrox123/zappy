import socket


class Connection:
    def __init__(self, sock: socket.socket) -> None:
        self._sock = sock
        self._buffer = b""

    @classmethod
    def connect(cls, hostname: str, port: int, timeout: float = 10.0) -> "Connection":
        sock = socket.create_connection((hostname, port), timeout=timeout)
        return cls(sock)

    def close(self) -> None:
        self._sock.close()

    def write_line(self, text: str) -> None:
        self._sock.sendall(f"{text}\n".encode())

    def readline(self, timeout: float | None = None) -> str:
        if timeout is not None:
            self._sock.settimeout(timeout)
        while b"\n" not in self._buffer:
            chunk = self._sock.recv(4096)
            if not chunk:
                raise ConnectionError("connection closed")
            self._buffer += chunk
        line, self._buffer = self._buffer.split(b"\n", 1)
        return line.decode().strip()
