"""Main game loop: perception, decision, action."""

import os
import sys

from config import AiConfig

from algo.actions.team_broadcasts import apply_team_broadcasts
from algo.client import ZappyClient, emit_status
from algo.decision.decide_next_action import decide_next_action
from algo.world.broadcast_messages import TeamBroadcast
from algo.world.inventory_parser import parse_inventory
from algo.world.look_parser import parse_look
from algo.world.map_model import Direction, MapModel, Orientation, Position
from algo.world.player_state import LeaderInfo, PlayerState, Role

LEVEL_REPORT_INTERVAL = 25
LOOK_INTERVAL = 3
INVENTORY_INTERVAL = 10
BROADCAST_INTERVAL = 6
FOOD_BUFFER = 8


def debug_log(message: str) -> None:
    print(f"[ZAPPY_DEBUG] {message}", file=sys.stderr, flush=True)


def read_role_from_environment() -> str:
    role = os.environ.get("ZAPPY_ROLE", "single")
    if role in ("leader", "follower"):
        return role
    return "single"


def parse_role(role_name: str) -> Role:
    if role_name == "follower":
        return Role.FOLLOWER
    return Role.LEADER


def play_until_dead(client: ZappyClient, role_name: str) -> bool:
    orientation = 0
    position = Position(0, 0)
    player = PlayerState(role=parse_role(role_name))
    map_model = MapModel(client.map_width, client.map_height)
    turns = 0
    last_look = ""
    inventory_known = False
    target_level = int(os.environ.get("ZAPPY_TARGET_LEVEL", "5"))

    while True:
        raw = client.drain_broadcasts()
        if raw:
            broadcasts = [TeamBroadcast(message=t, direction=d) for d, t in raw]
            apply_team_broadcasts(player, broadcasts)
            if os.environ.get("ZAPPY_DEBUG") == "1":
                for item in broadcasts:
                    debug_log(f"rx broadcast k={item.direction} msg={item.message}")

        need_inventory = turns % INVENTORY_INTERVAL == 0

        if need_inventory:
            inv_response = client.query_inventory()
            if inv_response == "dead":
                return True
            if inv_response != "ko" and inv_response.startswith("["):
                player.update_inventory(parse_inventory(inv_response))
                inventory_known = True

        need_look = turns % LOOK_INTERVAL == 0 or not last_look
        if (
            player.role == Role.LEADER
            and player.rally_position is not None
            and position == player.rally_position
            and player.ceremony_stones_on_tile
        ):
            need_look = True

        if need_look:
            look_response = client.query_look()
            if look_response == "dead":
                return True
            if look_response != "ko" and look_response.startswith("["):
                last_look = look_response
                tiles = parse_look(look_response)
                player.last_look_tiles = tiles
                map_model.apply_look(position, Orientation(orientation), tiles)

        previous_level = player.level
        player.level = client.level
        if client.level > previous_level:
            player.ready_level = None
            player.leader_info = None
            player.ceremony_stones_on_tile.clear()
            player.ready_followers.clear()
            player.pos_broadcast_sent = False
            # Invalidate queued moves: the old path is stale after leveling up
            player.pending_moves.clear()
            if player.role == Role.LEADER:
                player.rally_position = position

        player.position = position
        player.orientation = Orientation(orientation)

        if player.inventory_timer <= 0:
            player.inventory_timer = 20
            action = "Inventory"
            pending_move = None
        else:
            player.inventory_timer -= 1
            pending_move = player.pop_pending_move()
            if pending_move is not None:
                action = pending_move
            else:
                action = decide_next_action(player, map_model, target_level)

        debug_log(
            f"role={player.role.value} level={client.level} "
            f"pos=({position.x},{position.y}) food={player.food_count()} action={action}"
        )

        if action.startswith("Broadcast "):
            text = action[len("Broadcast ") :]
            response = client.send_broadcast(text)
        else:
            response = client.send_action(action)

            if response == "ko" and action.startswith("Take "):
                resource = action.split(" ", 1)[1]
                map_model.remove_resource(position, resource)

            if response != "ko":
                if action == "Look" and response.startswith("["):
                    last_look = response
                    tiles = parse_look(response)
                    player.last_look_tiles = tiles
                    map_model.apply_look(position, Orientation(orientation), tiles)
                if action == "Inventory" and response.startswith("["):
                    player.update_inventory(parse_inventory(response))
                    inventory_known = True
                if action in ("Forward", "Right", "Left"):
                    if player.leader_info is not None:
                        player.leader_info = LeaderInfo(
                            -1, player.leader_info.level, player.leader_info.uuid
                        )
                if action == "Forward" and response == "ok":
                    position = map_model.move(position, Orientation(orientation))
                    last_look = ""
                    player.last_look_tiles.clear()
                if action == "Right" and response == "ok":
                    orientation = int(
                        map_model.turn(Orientation(orientation), Direction.RIGHT)
                    )
                    player.last_look_tiles.clear()
                if action == "Left" and response == "ok":
                    orientation = int(
                        map_model.turn(Orientation(orientation), Direction.LEFT)
                    )
                    player.last_look_tiles.clear()
                if action.startswith("Take ") and response == "ok":
                    resource = action.split(" ", 1)[1]
                    player.inventory[resource] = player.inventory.get(resource, 0) + 1
                    map_model.remove_resource(position, resource)
                if action.startswith("Set ") and response == "ok":
                    resource = action.split(" ", 1)[1]
                    if player.inventory.get(resource, 0) > 0:
                        player.inventory[resource] -= 1
                        map_model.add_resource(position, resource)
                    player.ceremony_stones_on_tile[resource] = (
                        player.ceremony_stones_on_tile.get(resource, 0) + 1
                    )

        if response == "dead":
            return True
        if response == "ko":
            if action.startswith("Take "):
                resource = action.split(" ", 1)[1]
                map_model.remove_resource(position, resource)
                if resource == "food":
                    player.last_look_tiles.clear()
            if action == "Incantation":
                player.ceremony_stones_on_tile.clear()
            turns += 1
            continue

        turns += 1
        if turns % LEVEL_REPORT_INTERVAL == 0:
            emit_status(client.level)


def run(config: AiConfig) -> int:
    role_name = read_role_from_environment()
    max_level = 1
    reported_level = 0

    def on_level(level: int) -> None:
        nonlocal max_level, reported_level
        if level > max_level:
            max_level = level
        if level != reported_level:
            reported_level = level
            emit_status(level)

    while True:
        client = ZappyClient(
            config.hostname,
            config.port,
            config.team_name,
            on_level=on_level,
        )
        try:
            client.connect()
            client.handshake()
        except ConnectionRefusedError:
            print("[!] Connexion impossible : serveur indisponible")
            return 0
        except OSError as err:
            print(f"[!] Connexion impossible : {err}")
            return 0
        except ValueError as err:
            print(f"[!] Connexion refusée : {err}")
            return 0

        emit_status(client.level)
        reported_level = client.level
        try:
            died = play_until_dead(client, role_name)
        except (ConnectionError, OSError) as err:
            print(f"[!] Connexion perdue : {err}")
            client.close()
            return 0
        finally:
            client.close()

        if died:
            emit_status(max_level, alive=False)
            print(f"[!] {role_name} mort au niveau {max_level}", flush=True)
        else:
            emit_status(max_level)

        if died:
            if role_name in ("leader", "follower"):
                return 0
            continue
        return 0
