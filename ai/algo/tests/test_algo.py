import os
import sys
import unittest
from io import StringIO
from pathlib import Path
from unittest.mock import MagicMock, patch

AI_DIR = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(AI_DIR))

from algo.actions.incantation import (
    can_start_incantation,
    ceremony_requirements_satisfied,
    partner_is_present,
)
from algo.actions.navigation import (
    is_on_leader_tile,
    navigate_toward_leader_position,
)
from algo.actions.team_broadcasts import apply_team_broadcasts
from algo.client import ZappyClient, format_status, parse_broadcast, parse_level
from algo.decision.decide_next_action import decide_next_action
from algo.decision.solo_gameplay import decide_solo_level_one_action
from algo.decision.team_gameplay import (
    decide_follower_team_action,
    decide_leader_team_action,
    decide_team_action,
)
from algo.runner import play_until_dead, read_role_from_environment, run
from algo.world.broadcast_messages import (
    TeamBroadcast,
    format_regroup_broadcast,
    format_ready_broadcast,
    parse_regroup_message,
    parse_ready_message,
)
from algo.world.look_parser import TileView
from algo.world.map_model import MapModel, Orientation, Position
from algo.world.player_state import LeaderInfo, PlayerState, Role
from config import AiConfig


class ParseLevelTests(unittest.TestCase):
    def test_parse_level_extracts_number(self) -> None:
        self.assertEqual(parse_level("Current level: 2"), 2)


class ParseBroadcastTests(unittest.TestCase):
    def test_parse_broadcast_with_space_after_comma(self) -> None:
        self.assertEqual(parse_broadcast("message 3, ready"), (3, "ready"))

    def test_parse_broadcast_without_space_after_comma(self) -> None:
        self.assertEqual(parse_broadcast("message 2,REGROUP_L2"), (2, "REGROUP_L2"))


class BroadcastMessageTests(unittest.TestCase):
    def test_position_message_uses_underscores(self) -> None:
        self.assertEqual(format_regroup_broadcast(2), "REGROUP_L2")
        self.assertEqual(parse_regroup_message("REGROUP_L2"), 2)

    def test_ready_message_uses_underscores(self) -> None:
        self.assertEqual(format_ready_broadcast(3), "READY_L3")
        self.assertEqual(parse_ready_message("READY_L3"), 3)


class ClientAsyncTests(unittest.TestCase):
    def setUp(self) -> None:
        self.client = ZappyClient("localhost", 4242, "team1")

    def test_handle_async_line_updates_level_without_printing(self) -> None:
        with patch("sys.stdout", new_callable=StringIO) as output:
            handled = self.client.handle_async_line("Current level: 3")
        self.assertTrue(handled)
        self.assertEqual(self.client.level, 3)
        self.assertEqual(output.getvalue(), "")

    def test_incantation_returns_after_current_level(self) -> None:
        with patch.object(self.client, "send") as send_mock, patch.object(
            self.client,
            "receive_raw",
            side_effect=["Elevation underway", "Current level: 2"],
        ), patch("sys.stdout", new_callable=StringIO):
            response = self.client.send_action("Incantation")
        send_mock.assert_called_once_with("Incantation")
        self.assertEqual(response, "ok")
        self.assertEqual(self.client.level, 2)


class SoloGameplayTests(unittest.TestCase):
    def test_incant_when_linemate_on_tile(self) -> None:
        player = PlayerState()
        map_model = MapModel(10, 10)
        map_model.tiles[player.position] = ["player", "linemate"]
        player.last_look_tiles = [TileView(index=0, objects=("player", "linemate"))]
        self.assertEqual(decide_solo_level_one_action(player, map_model), "Incantation")

    def test_set_linemate_from_inventory(self) -> None:
        player = PlayerState(inventory={"linemate": 1})
        map_model = MapModel(10, 10)
        # Need to see only 1 player (self) on tile for Set to trigger
        player.last_look_tiles = [TileView(index=0, objects=("player",))]
        map_model.tiles[player.position] = ["player"]
        self.assertEqual(
            decide_solo_level_one_action(player, map_model), "Set linemate"
        )

    def test_solo_explores_when_no_linemate_known(self) -> None:
        player = PlayerState(level=1, inventory={"food": 10})
        player.last_look_tiles = []
        map_model = MapModel(10, 10)
        map_model.visited.add(player.position)
        action = decide_solo_level_one_action(player, map_model)
        self.assertEqual(action, "Look")


class TeamGameplayTests(unittest.TestCase):
    def test_leader_incants_when_ceremony_ready(self) -> None:
        player = PlayerState(level=2, role=Role.LEADER, inventory={"food": 8})
        player.rally_position = Position(0, 0)
        player.position = Position(0, 0)
        player.ceremony_stones_on_tile = {"linemate": 1, "deraumere": 1, "sibur": 1}
        player.broadcast_timer = 3  # Non-zero to skip broadcast phase
        player.last_look_tiles = [
            TileView(
                index=0, objects=("linemate", "deraumere", "sibur", "player", "player")
            )
        ]
        map_model = MapModel(10, 10)
        action = decide_leader_team_action(player, map_model, 3)
        self.assertEqual(action, "Incantation")

    def test_apply_position_broadcast_sets_leader_info(self) -> None:
        player = PlayerState(role=Role.FOLLOWER)
        apply_team_broadcasts(
            player,
            [TeamBroadcast(message="REGROUP_L2", direction=4)],
        )
        self.assertIsNotNone(player.leader_info)
        self.assertEqual(player.leader_info.direction, 4)
        self.assertEqual(player.leader_info.level, 2)

    def test_follower_waits_on_leader_tile(self) -> None:
        player = PlayerState(
            level=2,
            role=Role.FOLLOWER,
            inventory={"food": 10, "linemate": 1, "deraumere": 1, "sibur": 1},
            leader_info=LeaderInfo(direction=0, level=2),
        )
        player.position = Position(0, 0)
        player.ceremony_stones_on_tile = {"linemate": 1, "deraumere": 1, "sibur": 1}
        player.last_look_tiles = [
            TileView(index=0, objects=("linemate", "deraumere", "sibur", "player"))
        ]
        action = decide_follower_team_action(player, MapModel(10, 10), 8)
        # Only 1 player visible -> can't incant, so Look
        self.assertEqual(action, "Look")

    def test_extra_follower_explores(self) -> None:
        player = PlayerState(level=2, role=Role.FOLLOWER, inventory={"food": 10})
        map_model = MapModel(10, 10)
        action = decide_team_action(player, map_model, 8)
        # Follower with no leader_info gathers stones or explores
        self.assertIn(action, ("Look", "Forward", "Left", "Right"))

    def test_follower_moves_to_leader_when_carrying_stones(self) -> None:
        player = PlayerState(
            level=2,
            role=Role.FOLLOWER,
            inventory={"linemate": 1, "deraumere": 1, "sibur": 1, "food": 10},
            position=Position(2, 0),
            orientation=Orientation.EAST,
            leader_info=LeaderInfo(direction=3, level=2),
        )
        action = decide_follower_team_action(player, MapModel(10, 10), 8)
        self.assertIn(action, ("Forward", "Left", "Right"))

    def test_follower_gathers_stones_when_missing(self) -> None:
        player = PlayerState(
            level=2,
            role=Role.FOLLOWER,
            inventory={"food": 10},
            position=Position(0, 0),
            leader_info=LeaderInfo(direction=0, level=2),
        )
        # On leader tile but no stones → walks away to gather
        action = decide_follower_team_action(player, MapModel(10, 10), 8)
        self.assertEqual(action, "Forward")

    def test_leader_gathers_when_missing_stones(self) -> None:
        player = PlayerState(level=2, role=Role.LEADER, inventory={"food": 10})
        player.rally_position = Position(1, 8)
        player.position = Position(1, 8)
        player.last_look_tiles = [TileView(index=0, objects=("player",))]
        map_model = MapModel(10, 10)
        action = decide_leader_team_action(player, map_model, 8)
        # Leader needs stones, should explore (Look or movement)
        self.assertIn(action, ("Forward", "Left", "Right", "Look"))


class IncantationTests(unittest.TestCase):
    def test_count_players_includes_self(self) -> None:
        player = PlayerState()
        # "player" in the look data represents what the server reports.
        # The current player is counted as one "player" string.
        self.assertEqual(player.count_players_on_tile(["player"]), 1)

    def test_partner_requires_visible_player(self) -> None:
        player = PlayerState(
            position=Position(0, 0),
            leader_info=LeaderInfo(direction=0, level=2),
        )
        player.last_look_tiles = [TileView(index=0, objects=())]
        map_model = MapModel(10, 10)
        self.assertFalse(partner_is_present(player, map_model, 3))

    def test_can_incantate_with_partner_and_stones(self) -> None:
        player = PlayerState(level=2, inventory={"food": 5})
        player.last_look_tiles = [
            TileView(
                index=0, objects=("linemate", "deraumere", "sibur", "player", "player")
            )
        ]
        map_model = MapModel(10, 10)
        self.assertTrue(can_start_incantation(player, map_model, 3, 3))

    def test_ceremony_requires_all_stones(self) -> None:
        player = PlayerState(level=2, role=Role.LEADER, inventory={"food": 5})
        player.position = Position(0, 0)
        player.last_look_tiles = [
            TileView(index=0, objects=("linemate", "deraumere", "player", "player"))
        ]
        map_model = MapModel(10, 10)
        self.assertFalse(ceremony_requirements_satisfied(player, map_model, 3))


class NavigationTests(unittest.TestCase):
    def test_on_leader_tile_when_direction_zero(self) -> None:
        player = PlayerState(
            position=Position(0, 0),
            leader_info=LeaderInfo(direction=0, level=2),
        )
        self.assertTrue(is_on_leader_tile(player))

    def test_navigate_toward_leader_uses_direction(self) -> None:
        player = PlayerState(
            position=Position(2, 0),
            orientation=Orientation.EAST,
            leader_info=LeaderInfo(direction=3, level=2),
        )
        action = navigate_toward_leader_position(player, MapModel(10, 10))
        self.assertIn(action, ("Forward", "Left", "Right"))


class RunnerTests(unittest.TestCase):
    def test_read_role_defaults_single(self) -> None:
        with patch.dict(os.environ, {}, clear=True):
            self.assertEqual(read_role_from_environment(), "single")

    def test_run_returns_zero_when_server_unavailable(self) -> None:
        config = AiConfig(port=1, team_name="team1", hostname="127.0.0.1")
        with patch.object(ZappyClient, "connect", side_effect=ConnectionRefusedError):
            self.assertEqual(run(config), 0)

    def test_play_until_dead_uses_decision_layer(self) -> None:
        client = MagicMock()
        client.level = 1
        client.map_width = 10
        client.map_height = 10
        client.drain_broadcasts.return_value = []
        client.query_inventory.return_value = "[ food 10, linemate 0, deraumere 0, sibur 0, mendiane 0, phiras 0, thystame 0 ]"
        client.query_look.return_value = "[ linemate, ]"
        client.send_action.side_effect = ["ok", "dead"]
        with patch(
            "algo.runner.decide_next_action",
            side_effect=["Take linemate", "Forward"],
        ):
            self.assertTrue(play_until_dead(client, "single"))
        client.send_action.assert_any_call("Take linemate")

    def test_death_status_format(self) -> None:
        self.assertEqual(format_status(2, alive=False), "level=2 alive=False")


class DecideNextActionTests(unittest.TestCase):
    def test_level_one_uses_solo_logic(self) -> None:
        player = PlayerState(level=1)
        map_model = MapModel(10, 10)
        map_model.tiles[player.position] = ["player", "linemate"]
        player.last_look_tiles = [TileView(index=0, objects=("player", "linemate"))]
        self.assertEqual(decide_next_action(player, map_model, 8), "Incantation")

    def test_follower_level_one_stays_solo(self) -> None:
        player = PlayerState(level=1, role=Role.FOLLOWER)
        map_model = MapModel(10, 10)
        map_model.tiles[player.position] = ["player", "linemate"]
        player.last_look_tiles = [TileView(index=0, objects=("player", "linemate"))]
        self.assertEqual(decide_next_action(player, map_model, 8), "Incantation")

    def test_follower_level_two_uses_team_logic(self) -> None:
        player = PlayerState(
            level=2,
            role=Role.FOLLOWER,
            leader_info=LeaderInfo(direction=3, level=2),
            inventory={"food": 10},
        )
        map_model = MapModel(10, 10)
        action = decide_next_action(player, map_model, 8)
        self.assertIn(action, ("Forward", "Left", "Right", "Look", "Inventory"))


if __name__ == "__main__":
    unittest.main()
