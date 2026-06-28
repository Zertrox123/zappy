"""Apply incoming team broadcasts and ceremony stone accounting."""

from algo.world.broadcast_messages import (
    TeamBroadcast,
    parse_regroup_message,
)
from algo.world.player_state import LeaderInfo, PlayerState, Role


def apply_team_broadcasts(
    player: PlayerState,
    broadcasts: list[TeamBroadcast],
) -> None:
    for item in broadcasts:
        regroup_result = parse_regroup_message(item.message)
        if regroup_result is not None:
            if isinstance(regroup_result, tuple):
                level, sender_uuid = regroup_result
            else:
                level, sender_uuid = regroup_result, ""

            if level == player.level + 1:
                if (
                    player.role == Role.FOLLOWER
                    and player.leader_info is not None
                    and player.leader_info.level == level
                    and player.leader_info.direction == 0
                    and sender_uuid != player.leader_info.uuid
                    and sender_uuid != ""
                ):
                    continue

                player.leader_info = LeaderInfo(
                    direction=item.direction,
                    level=level,
                    uuid=sender_uuid,
                )
                player.ticks_since_leader = 0

                if item.direction == 0:
                    player.pending_moves.clear()

                if player.role == Role.LEADER:
                    if not sender_uuid or sender_uuid > player.uuid:
                        player.role = Role.FOLLOWER
                        player.pending_moves.clear()
