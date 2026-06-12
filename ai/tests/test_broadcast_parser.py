import unittest

from world.broadcast_parser import (
    parse_broadcast,
    parse_have_message,
    parse_hint_message,
    parse_need_message,
    parse_pos_message,
    parse_ready_message,
    parse_wait_message,
)


class BroadcastParserTests(unittest.TestCase):
    def test_parse_broadcast_line(self) -> None:
        item = parse_broadcast("POS 3 4 L2, 5")
        self.assertIsNotNone(item)
        assert item is not None
        self.assertEqual(item.message, "POS 3 4 L2")
        self.assertEqual(item.direction, 5)

    def test_parse_pos_message(self) -> None:
        self.assertEqual(parse_pos_message("POS 3 4 L2"), (3, 4, 2))

    def test_parse_ready_message(self) -> None:
        self.assertEqual(parse_ready_message("READY L3"), 3)

    def test_parse_need_message(self) -> None:
        self.assertEqual(parse_need_message("NEED L3 deraumere"), (3, "deraumere"))

    def test_parse_have_message(self) -> None:
        self.assertEqual(parse_have_message("HAVE sibur 1"), ("sibur", 1))
        self.assertEqual(parse_have_message("HAVE linemate"), ("linemate", 1))

    def test_parse_hint_message(self) -> None:
        self.assertEqual(parse_hint_message("HINT 3 4 linemate"), (3, 4, "linemate"))

    def test_parse_wait_message(self) -> None:
        self.assertTrue(parse_wait_message("WAIT"))
        self.assertFalse(parse_wait_message("POS 1 2 L2"))

    def test_invalid_broadcast_returns_none(self) -> None:
        self.assertIsNone(parse_broadcast("hello world"))


if __name__ == "__main__":
    unittest.main()
