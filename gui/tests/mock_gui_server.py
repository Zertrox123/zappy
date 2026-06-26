#!/usr/bin/env python3

import socket
import sys
import time


def main() -> int:
    listener = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    listener.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    listener.bind(("127.0.0.1", 0))
    listener.listen(1)
    port = listener.getsockname()[1]
    listener.settimeout(5.0)

    print(port, flush=True)

    try:
        conn, _addr = listener.accept()
        conn.settimeout(2.0)
        conn.sendall(b"WELCOME\n")

        buffer = b""
        while b"\n" not in buffer:
            chunk = conn.recv(4096)
            if not chunk:
                return 1
            buffer += chunk

        line = buffer.split(b"\n", 1)[0].decode()
        if line != "GRAPHIC":
            return 1

        conn.sendall(b"msz 10 10\n")
        time.sleep(0.1)
        conn.close()
        return 0
    except OSError:
        return 1
    finally:
        listener.close()


if __name__ == "__main__":
    raise SystemExit(main())
