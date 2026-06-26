"""Single entry point for choosing the next server command."""

from algo.actions.exploration import decide_exploration_action
from algo.actions.survival import decide_survival_action
from algo.decision.solo_gameplay import decide_solo_level_one_action
from algo.decision.team_gameplay import decide_team_action
from algo.world.map_model import MapModel
from algo.world.player_state import PlayerState, Role


def decide_next_action(
    player: PlayerState,
    map_model: MapModel,
    target_level: int,
) -> str:
    if player.level < 2:
        solo = decide_solo_level_one_action(player, map_model)
        if solo is not None:
            return solo
        survival = decide_survival_action(player, map_model)
        if survival is not None:
            return survival
        return decide_exploration_action(player, map_model)

    if player.role in (Role.LEADER, Role.FOLLOWER) and player.level >= 2:
        return decide_team_action(player, map_model, target_level)

    survival = decide_survival_action(player, map_model)
    if survival is not None:
        return survival
    return decide_exploration_action(player, map_model)
