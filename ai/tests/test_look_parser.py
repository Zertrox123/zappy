import unittest

from world.look_parser import parse_look


class LookParserTests(unittest.TestCase):
    def test_parse_single_tile(self) -> None:
        tiles = parse_look("[ player food ]")
        self.assertEqual(len(tiles), 1)
        self.assertEqual(tiles[0].objects, ("player", "food"))

    def test_parse_reference_style_look(self) -> None:
        tiles = parse_look("[ player food, sibur, linemate phiras, linemate ]")
        self.assertEqual(len(tiles), 4)
        self.assertEqual(tiles[0].objects, ("player", "food"))
        self.assertEqual(tiles[1].objects, ("sibur",))
        self.assertEqual(tiles[2].objects, ("linemate", "phiras"))
        self.assertEqual(tiles[3].objects, ("linemate",))

    def test_parse_multiple_tiles(self) -> None:
        tiles = parse_look("[ food, , linemate ]")
        self.assertEqual(len(tiles), 3)
        self.assertEqual(tiles[0].objects, ("food",))
        self.assertEqual(tiles[1].objects, ())
        self.assertEqual(tiles[2].objects, ("linemate",))

    def test_parse_tile_with_multiple_objects(self) -> None:
        tiles = parse_look("[ player linemate ]")
        self.assertEqual(len(tiles), 1)
        self.assertEqual(tiles[0].objects, ("player", "linemate"))

    def test_parse_empty(self) -> None:
        tiles = parse_look("[]")
        self.assertEqual(tiles, [])

    def test_invalid_payload_raises(self) -> None:
        with self.assertRaises(ValueError):
            parse_look("food")


if __name__ == "__main__":
    unittest.main()
