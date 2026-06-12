use server::server::ClientHandler;

use crate::data::{Entity, EntityId, Map};

pub struct Game {
    map: Map,
    players: Vec<Entity>,
    teams: Vec<String>,
    clients_per_team: usize,
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

    pub fn run_ticks(&mut self) {}

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
}

impl ClientHandler for Game {
    fn tick(&mut self) {
        self.run_ticks();
    }

    fn new_client(&mut self) -> u64 {
        self.add_players() as u64
    }

    fn client_message(&mut self, id: u64, data: &str) -> Vec<u8> {
        match crate::data::parse(data) {
            Ok(action) if self.add_action_to_player(id as EntityId, action) => b"ok\n".to_vec(),
            _ => b"ko\n".to_vec(),
        }
    }

    fn client_disconnect(&mut self, _id: u64) {}
}
