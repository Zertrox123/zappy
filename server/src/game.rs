use std::usize;
use rand::Rng;

use crate::data::{Entity, EntityId, Map};

#[derive(Clone, Debug)]
pub struct Game {
    map: Map,
    players: Vec<Entity>,
}

impl Default for Game {
    fn default() -> Self {
        Self::new()
    }

}

impl Game {
    pub fn new() -> Self {
        let mut map = Map::new(2,2);
        map.populate();
        Game {
            map: map,
            players: Vec::new(),
        }
    }

    pub fn run_ticks(&mut self) {}

    pub fn add_players(&mut self) -> EntityId {
        let id = self.players.len();
        self.players.push(Entity::new_dummy());
        self.players[id].setId(id);
        guj
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
