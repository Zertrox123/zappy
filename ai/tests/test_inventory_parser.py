import unittest

from world.inventory_parser import parse_inventory


class InventoryParserTests(unittest.TestCase):
    def test_parse_inventory(self) -> None:
        inv = parse_inventory("[ food 10, linemate 1, deraumere 0 ]")
        self.assertEqual(inv["food"], 10)
        self.assertEqual(inv["linemate"], 1)
        self.assertEqual(inv["deraumere"], 0)
        self.assertEqual(inv["sibur"], 0)

    def test_parse_empty_inventory(self) -> None:
        inv = parse_inventory("[]")
        self.assertEqual(inv["food"], 0)

    def test_invalid_item_raises(self) -> None:
        with self.assertRaises(ValueError):
            parse_inventory("[ food ]")


if __name__ == "__main__":
    unittest.main()
