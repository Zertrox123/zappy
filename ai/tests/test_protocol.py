import unittest

from client.command_queue import (
    CommandBufferFullError,
    CommandInFlightError,
    CommandQueue,
)
from client.connection import Connection
from client.protocol import CommandStatus, ProtocolClient, ProtocolError
from tests.mocks.mock_server import MockServer


class ProtocolTests(unittest.TestCase):
    def test_handshake_success(self) -> None:
        server = MockServer(
            team_name="team",
            script=[
                ("send", "WELCOME"),
                ("recv", "team"),
                ("send", "4"),
                ("send", "10 10"),
            ],
        )
        port = server.start()
        try:
            conn = Connection.connect("127.0.0.1", port)
            client = ProtocolClient(conn)
            info = client.handshake("team")
            self.assertEqual(info.available_slots, 4)
            self.assertEqual(info.width, 10)
            self.assertEqual(info.height, 10)
            conn.close()
        finally:
            server.stop()

    def test_look_returns_payload(self) -> None:
        server = MockServer(
            script=[
                ("send", "WELCOME"),
                ("recv", "team"),
                ("send", "1"),
                ("send", "3 3"),
                ("recv", "Look"),
                ("send", "ok"),
                ("send", "[ food linemate ]"),
            ],
        )
        port = server.start()
        try:
            conn = Connection.connect("127.0.0.1", port)
            client = ProtocolClient(conn)
            client.handshake("team")
            response = client.look()
            self.assertEqual(response.status, CommandStatus.OK)
            self.assertEqual(response.payload, "[ food linemate ]")
            conn.close()
        finally:
            server.stop()

    def test_look_ref_style_direct_payload(self) -> None:
        server = MockServer(
            script=[
                ("send", "WELCOME"),
                ("recv", "team"),
                ("send", "1"),
                ("send", "3 3"),
                ("recv", "Look"),
                ("send", "[ player food, sibur, linemate ]"),
            ],
        )
        port = server.start()
        try:
            conn = Connection.connect("127.0.0.1", port)
            client = ProtocolClient(conn)
            client.handshake("team")
            response = client.look()
            self.assertEqual(response.status, CommandStatus.OK)
            self.assertEqual(response.payload, "[ player food, sibur, linemate ]")
            conn.close()
        finally:
            server.stop()

    def test_inventory_ref_style_direct_payload(self) -> None:
        server = MockServer(
            script=[
                ("send", "WELCOME"),
                ("recv", "team"),
                ("send", "1"),
                ("send", "3 3"),
                ("recv", "Inventory"),
                (
                    "send",
                    "[ food 9, linemate 0, deraumere 0, sibur 0, mendiane 0, phiras 0, thystame 0 ]",
                ),
            ],
        )
        port = server.start()
        try:
            conn = Connection.connect("127.0.0.1", port)
            client = ProtocolClient(conn)
            client.handshake("team")
            response = client.inventory()
            self.assertEqual(response.status, CommandStatus.OK)
            self.assertIn("food 9", response.payload or "")
            conn.close()
        finally:
            server.stop()

    def test_look_skips_unsolicited_broadcast(self) -> None:
        server = MockServer(
            script=[
                ("send", "WELCOME"),
                ("recv", "team"),
                ("send", "1"),
                ("send", "3 3"),
                ("recv", "Look"),
                ("send", "Elevation underway"),
                ("send", "[ food ]"),
            ],
        )
        port = server.start()
        try:
            conn = Connection.connect("127.0.0.1", port)
            client = ProtocolClient(conn)
            client.handshake("team")
            response = client.look()
            self.assertEqual(response.status, CommandStatus.OK)
            self.assertEqual(response.payload, "[ food ]")
            conn.close()
        finally:
            server.stop()

    def test_broadcast_enqueued_on_unsolicited_line(self) -> None:
        server = MockServer(
            script=[
                ("send", "WELCOME"),
                ("recv", "team"),
                ("send", "1"),
                ("send", "3 3"),
                ("recv", "Look"),
                ("send", "POS 2 3 L2, 4"),
                ("send", "[ food ]"),
            ],
        )
        port = server.start()
        try:
            conn = Connection.connect("127.0.0.1", port)
            client = ProtocolClient(conn)
            client.handshake("team")
            response = client.look()
            self.assertEqual(response.status, CommandStatus.OK)
            queued = client.drain_broadcasts()
            self.assertEqual(len(queued), 1)
            self.assertEqual(queued[0].message, "POS 2 3 L2")
            self.assertEqual(queued[0].direction, 4)
            conn.close()
        finally:
            server.stop()

    def test_unknown_command_returns_ko(self) -> None:
        server = MockServer(
            script=[
                ("send", "WELCOME"),
                ("recv", "team"),
                ("send", "1"),
                ("send", "3 3"),
                ("recv", "Jump"),
                ("send", "ko"),
            ],
        )
        port = server.start()
        try:
            conn = Connection.connect("127.0.0.1", port)
            client = ProtocolClient(conn)
            client.handshake("team")
            response = client.send_command("Jump")
            self.assertEqual(response.status, CommandStatus.KO)
            conn.close()
        finally:
            server.stop()

    def test_handshake_invalid_welcome_raises(self) -> None:
        server = MockServer(script=[("send", "HELLO")])
        port = server.start()
        try:
            conn = Connection.connect("127.0.0.1", port)
            client = ProtocolClient(conn)
            with self.assertRaises(ProtocolError):
                client.handshake("team")
            conn.close()
        finally:
            server.stop()


class CommandQueueTests(unittest.TestCase):
    def test_single_flight_enforced(self) -> None:
        queue = CommandQueue()
        queue.acquire()
        with self.assertRaises(CommandInFlightError):
            queue.acquire()

    def test_buffer_full(self) -> None:
        queue = CommandQueue()
        queue._pending = CommandQueue.MAX_PENDING
        with self.assertRaises(CommandBufferFullError):
            queue.acquire()


if __name__ == "__main__":
    unittest.main()
