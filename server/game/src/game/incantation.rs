use crate::action::Action;
use crate::data::{EntityId, Position, RESOURCES};
use server::server::ClientReply;

use super::{Game, WIN_LEVEL, WIN_PLAYERS};

impl Game {
    pub(super) fn incantation_requirement(level: usize) -> Option<(usize, [usize; 7])> {
        Some(match level {
            1 => (1, [0, 1, 0, 0, 0, 0, 0]),
            2 => (2, [0, 1, 1, 1, 0, 0, 0]),
            3 => (2, [0, 2, 0, 1, 0, 2, 0]),
            4 => (4, [0, 1, 1, 2, 0, 1, 0]),
            5 => (4, [0, 1, 2, 1, 3, 0, 0]),
            6 => (6, [0, 1, 2, 3, 0, 1, 0]),
            7 => (6, [0, 2, 2, 2, 2, 2, 1]),
            _ => return None,
        })
    }

    fn incantation_who(&self, position: Position, level: usize) -> Vec<EntityId> {
        self.players
            .iter()
            .filter(|player| {
                player.raw_fd() >= 0
                    && player.level() == level
                    && self.same_tile(player.position(), position)
            })
            .map(|player| player.get_id() as EntityId)
            .collect()
    }

    fn incantation_ready(&self, position: Position, level: usize) -> Option<Vec<EntityId>> {
        let (player_count, resources) = Self::incantation_requirement(level)?;
        let participants = self.incantation_who(position, level);
        if participants.len() < player_count {
            return None;
        }

        let tile = self.map.get(position.x as isize, position.y as isize);
        for resource in RESOURCES {
            let available = tile
                .resources()
                .iter()
                .filter(|item| **item == resource)
                .count();
            if available < resources[resource as usize] {
                return None;
            }
        }

        Some(participants)
    }

    pub(super) fn start_incantation(&mut self, player_index: usize) -> Option<ClientReply> {
        let position = self.players[player_index].position();
        let level = self.players[player_index].level();
        let participants = self.incantation_ready(position, level)?;

        for participant in &participants {
            self.players[*participant as usize].actions.clear();
        }
        self.players[player_index].add_action(Action::new_incantation(
            position,
            level,
            participants.clone(),
        ));
        self.frozen.extend(participants.iter().copied());
        let players = participants
            .iter()
            .map(|id| format!("#{id}"))
            .collect::<Vec<_>>()
            .join(" ");
        let (x, y) = self.gui_position_xy(position);
        self.send_to_guis(&format!("pic {} {} {} {}\n", x, y, level, players));

        Some(ClientReply::data(b"Elevation underway\n".to_vec()))
    }

    pub(super) fn finish_incantation(
        &mut self,
        position: Position,
        level: usize,
        participants: &[EntityId],
    ) -> String {
        let Some(current_participants) = self.incantation_ready(position, level) else {
            self.frozen.retain(|id| !participants.contains(id));
            let (x, y) = self.gui_position_xy(position);
            self.send_to_guis(&format!("pie {x} {y} 0\n"));
            return "ko\n".into();
        };

        let mut level_events = String::new();
        for participant in participants {
            if current_participants.contains(participant) {
                self.players[*participant as usize].level_up();
                level_events.push_str(&self.gui_player_line("plv", *participant));
            }
        }

        let (_, resources) = Self::incantation_requirement(level).unwrap();
        for resource in RESOURCES {
            for _ in 0..resources[resource as usize] {
                self.map.take(position, resource);
            }
        }

        self.frozen.retain(|id| !participants.contains(id));
        let (x, y) = self.gui_position_xy(position);
        let mut gui_events = format!(
            "pie {x} {y} 1\n{}{}",
            level_events,
            self.gui_tile_at(position)
        );
        if let Some(team) = self.winning_team() {
            gui_events.push_str(&format!("seg {team}\n"));
        }
        self.send_to_guis(&gui_events);
        format!("Current level: {}\n", level + 1)
    }

    pub(super) fn winning_team(&self) -> Option<String> {
        self.teams.iter().find_map(|team| {
            let winners = self
                .players
                .iter()
                .filter(|player| player.team() == team && player.level() >= WIN_LEVEL)
                .count();
            (winners >= WIN_PLAYERS).then(|| team.clone())
        })
    }
}
