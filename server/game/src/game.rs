use std::collections::HashMap;

use server::server::{ClientHandler, ClientReply};

use crate::data::{Entity, EntityId, Map, Resource};

const REFILL_INTERVAL: u64 = 20;

enum SessionState {
    AwaitingTeamName,
    Ready {
        player_id: EntityId,
        team: String,
    },
}

struct Session {
    state: SessionState,
}

pub struct Game {
    map: Map,
    players: Vec<Entity>,
    teams: Vec<String>,
    clients_per_team: usize,
    ticks: u64,
    sessions: HashMap<u64, Session>,
    next_session_id: u64,
}

impl Game {
    pub fn new(width: usize, height: usize, teams: Vec<String>, clients_per_team: usize) -> Self {
        let mut map = Map::new(width, height);
        map.populate();
        Game {
            map,
            players: Vec::new(),
            teams,
            clients_per_team,
            ticks: 0,
            sessions: HashMap::new(),
            next_session_id: 0,
        }
    }

    pub fn map_dimensions(&self) -> (usize, usize) {
        self.map.dimensions()
    }

    pub fn teams(&self) -> &[String] {
        &self.teams
    }

    pub fn clients_per_team(&self) -> usize {
        self.clients_per_team
    }

    pub fn run_ticks(&mut self) {
        self.ticks += 1;
        if self.ticks % REFILL_INTERVAL == 0 {
            self.map.refill();
        }
    }

    pub fn deplete(&mut self, resource: Resource, amount: usize) -> usize {
        self.map.deplete(resource, amount)
    }

    pub fn count(&self, resource: Resource) -> usize {
        self.map.count(resource)
    }

    pub fn add_players(&mut self) -> EntityId {
        let id = self.players.len();
        self.players.push(Entity::new_dummy());
        self.players[id].set_id(id);
        id as EntityId
    }

    fn add_action_to_player(&mut self, id: EntityId, action: crate::action::Action) -> bool {
        self.players
            .get_mut(id as usize)
            .is_some_and(|p| p.add_action(action))
    }

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

    fn accept_team_name(&mut self, session_id: u64, team_name: &str) -> ClientReply {
        if !self.teams.iter().any(|team| team == team_name) {
            self.sessions.remove(&session_id);
            return ClientReply::data_then_close(b"ko\n".to_vec());
        }

        let available = self
            .clients_per_team
            .saturating_sub(self.connected_on_team(team_name));
        if available < 1 {
            self.sessions.remove(&session_id);
            return ClientReply::data_then_close(b"ko\n".to_vec());
        }

        let player_id = self.add_players();
        self.players[player_id as usize].set_team(&team_name.to_string());
        let (width, height) = self.map_dimensions();
        self.sessions.insert(
            session_id,
            Session {
                state: SessionState::Ready {
                    player_id,
                    team: team_name.to_string(),
                },
            },
        );

        ClientReply::data(format!("{available}\n{width} {height}\n").into_bytes())
    }
}

impl ClientHandler for Game {
    fn tick(&mut self) {
        self.run_ticks();
    }

    fn on_connect(&mut self) -> (u64, Vec<u8>) {
        let session_id = self.next_session_id;
        self.next_session_id += 1;
        self.sessions.insert(
            session_id,
            Session {
                state: SessionState::AwaitingTeamName,
            },
        );
        (session_id, b"WELCOME\n".to_vec())
    }

    fn client_message(&mut self, id: u64, data: &str) -> ClientReply {
        let Some(session) = self.sessions.get(&id) else {
            return ClientReply::data(b"ko\n".to_vec());
        };

        match &session.state {
            SessionState::AwaitingTeamName => self.accept_team_name(id, data),
            SessionState::Ready { player_id, .. } => {
                let player_id = *player_id;
                match crate::data::parse(data) {
                    Ok(action) if self.add_action_to_player(player_id, action) => {
                        ClientReply::data(b"ok\n".to_vec())
                    }
                    _ => ClientReply::data(b"ko\n".to_vec()),
                }
            }
        }
    }

    fn client_disconnect(&mut self, id: u64) {
        self.sessions.remove(&id);
    }
}
