use std::collections::HashMap;

use crate::data::{EntityId, parse};
use server::server::{ClientHandler, ClientReply};

use super::{Game, MAX_ACTION, Session, SessionState};

impl Game {
    fn connected_on_team(&self, team: &str) -> usize {
        self.sessions
            .values()
            .filter(|session| {
                matches!(
                    &session.state,
                    SessionState::Ready { team: name, .. } if name == team
                )
            })
            .count()
    }

    fn available_slots(&self, team: &str) -> usize {
        self.team_capacity[team].saturating_sub(self.connected_on_team(team))
    }

    fn accept_team_name(&mut self, client_fd: u64, team_name: &str) -> ClientReply {
        if !self.teams.iter().any(|team| team == team_name) {
            self.sessions.remove(&client_fd);
            return ClientReply::data_then_close(b"ko\n".to_vec());
        }

        let connected = self.connected_on_team(team_name);
        let available = self.available_slots(team_name);
        if available < 1 {
            self.sessions.remove(&client_fd);
            return ClientReply::data_then_close(b"ko\n".to_vec());
        }

        let player_id = self.add_players();
        self.players[player_id as usize].set_raw_fd(client_fd as i32);
        self.players[player_id as usize].set_team(&team_name.to_string());
        let mut egg_id = None;
        if connected >= self.clients_per_team
            && let Some(index) = self.eggs.iter().position(|egg| egg.team == team_name)
        {
            let egg = self.eggs.remove(index);
            egg_id = Some(egg.id);
            self.players[player_id as usize].set_position(egg.position);
        }
        let (width, height) = self.map_dimensions();
        self.sessions.insert(
            client_fd,
            Session {
                state: SessionState::Ready {
                    player_id,
                    team: team_name.to_string(),
                },
            },
        );
        let mut gui_msg = String::new();
        if let Some(egg_id) = egg_id {
            gui_msg.push_str(&format!("ebo #{egg_id}\n"));
        }
        gui_msg.push_str(&self.gui_player_line("pnw", player_id));
        self.send_to_guis(&gui_msg);

        ClientReply::data(format!("{available}\n{width} {height}\n").into_bytes())
    }

    pub(super) fn remove_player(&mut self, player_index: usize) {
        let removed_id = self.players[player_index].get_id() as EntityId;
        self.players[player_index].set_raw_fd(-1);
        self.players.remove(player_index);
        for (index, player) in self.players.iter_mut().enumerate() {
            player.set_id(index);
        }
        self.sessions.retain(|_, session| match &mut session.state {
            SessionState::Ready { player_id, .. } if *player_id == removed_id => false,
            SessionState::Ready { player_id, .. } => {
                if *player_id > removed_id {
                    *player_id -= 1;
                }
                true
            }
            _ => true,
        });
        self.frozen = self
            .frozen
            .iter()
            .filter_map(|id| match id.cmp(&removed_id) {
                std::cmp::Ordering::Less => Some(*id),
                std::cmp::Ordering::Equal => None,
                std::cmp::Ordering::Greater => Some(*id - 1),
            })
            .collect();
    }
}

impl ClientHandler for Game {
    fn tickrate(&self) -> Option<usize> {
        Some(self.time_unit)
    }

    fn tick(&mut self) -> HashMap<i32, String> {
        self.run_ticks();
        std::mem::take(&mut self.reply)
    }

    fn on_connect(&mut self, client_fd: u64) -> Vec<u8> {
        self.sessions.insert(
            client_fd,
            Session {
                state: SessionState::AwaitingTeamName,
            },
        );
        b"WELCOME\n".to_vec()
    }

    fn client_message(&mut self, client_fd: u64, data: &str) -> Option<ClientReply> {
        let Some(session) = self.sessions.get(&client_fd) else {
            return Some(ClientReply::data(b"ko\n".to_vec()));
        };

        match &session.state {
            SessionState::AwaitingTeamName if data == "GRAPHIC" => {
                self.sessions.insert(
                    client_fd,
                    Session {
                        state: SessionState::Gui,
                    },
                );
                Some(ClientReply::data(self.gui_initial_state().into_bytes()))
            }
            SessionState::AwaitingTeamName => Some(self.accept_team_name(client_fd, data)),
            SessionState::Gui => Some(self.gui_command(data)),
            SessionState::Ready { team, .. } if data == "Connect_nbr" => Some(ClientReply::data(
                format!("{}\n", self.available_slots(team)).into_bytes(),
            )),
            SessionState::Ready { player_id, .. } => match parse(data) {
                Ok(act) => {
                    if self.players[*player_id as usize].actions.len() >= MAX_ACTION {
                        return Some(ClientReply::data(b"ko\n".to_vec()));
                    }
                    if self.frozen.contains(player_id) {
                        return Some(ClientReply::data(b"ko\n".to_vec()));
                    }
                    if data == "Incantation" {
                        return self
                            .start_incantation(*player_id as usize)
                            .or_else(|| Some(ClientReply::data(b"ko\n".to_vec())));
                    }
                    self.players[*player_id as usize].add_action(act);
                    None
                }
                Err(_) => Some(ClientReply::data(b"ko\n".to_vec())),
            },
        }
    }

    fn client_disconnect(&mut self, client_fd: u64) {
        if let Some(Session {
            state: SessionState::Ready { player_id, .. },
        }) = self.sessions.remove(&client_fd)
        {
            self.players[player_id as usize].set_raw_fd(-1);
            self.send_to_guis(&format!("pdi #{player_id}\n"));
        }
    }
}
