use std::usize;

use crate::config::ServerConfig;
use crate::data::{Entity, EntityId, Map};

#[derive(Clone, Debug)]
pub struct Game {
    map: Map,
    players: Vec<Entity>,
    teams: Vec<String>,
    clients_per_team: usize,
}


impl Game {
    pub fn new(config: &ServerConfig) -> Self {
        let mut map = Map::new(config.width, config.height);
        map.populate();
        Game {
            map,
            players: Vec::new(),
            teams: config.teams.clone(),
            clients_per_team: config.clients_per_team,
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
        self.players[id].setId(id);
        id as EntityId
    }

    pub fn get_entity(&mut self, id: EntityId) -> Option<Entity> {
        let size = self.players.len();
        let id = id as usize;

        if id > size {
            return None;
        }
        None
    }
}
