#!/usr/bin/env python3
"""Minimal Zappy AI for GUI demos: connects and walks."""

import socket
import sys
import time


def read_line(sock: socket.socket, buf: bytearray) -> str:
    while b"\n" not in buf:
        chunk = sock.recv(4096)
        if not chunk:
            raise ConnectionError("server closed connection")
        buf.extend(chunk)
    line, rest = buf.split(b"\n", 1)
    buf.clear()
    buf.extend(rest)
    return line.decode().strip()


def main() -> int:
    if len(sys.argv) < 4:
        sys.stderr.write(
            "USAGE: demo_ai.py host port team [commands...]\n"
            "Example: demo_ai.py 127.0.0.1 4242 name1 Forward Right Forward\n"
        )
        return 84

    host = sys.argv[1]
    port = int(sys.argv[2])
    team = sys.argv[3]
    commands = sys.argv[4:] if len(sys.argv) > 4 else []

    sock = socket.create_connection((host, port), timeout=5)
    sock.settimeout(5.0)
    buf = bytearray()

    welcome = read_line(sock, buf)
    if welcome != "WELCOME":
        sys.stderr.write(f"unexpected handshake line: {welcome!r}\n")
        return 84

    sock.sendall(f"{team}\n".encode())
    slots = read_line(sock, buf)
    map_size = read_line(sock, buf)
    sys.stderr.write(f"[demo_ai] team={team} slots={slots} map={map_size}\n")

    cycle = commands or ["Forward", "Right", "Forward", "Left"]
    i = 0
    while True:
        cmd = cycle[i % len(cycle)]
        i += 1
        sock.sendall(f"{cmd}\n".encode())
        response = read_line(sock, buf)
        sys.stderr.write(f"[demo_ai] {cmd} -> {response}\n")
        if response == "dead":
            return 0
        time.sleep(0.15)


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (ConnectionError, OSError, TimeoutError) as err:
        sys.stderr.write(f"[demo_ai] error: {err}\n")
        raise SystemExit(84)
