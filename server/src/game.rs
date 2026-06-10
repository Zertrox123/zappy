use crate::data::{Entity, Map};

#[derive(Clone, Debug)]
pub struct Game {
    map: Map,
    players: Vec<Entity>,
}

impl Game {
    pub fn new() -> Self {
        Game {
            map: Map::new(50, 50),
            players: Vec::new(),
        }
    }

    pub fn run_ticks(&mut self) {
    }
    pub fn add_players(&mut self) -> usize {
        let id = self.players.len();
        self.players.push(Entity::new_dummy());
        self.players[id].setId(id);
        return id;
    }
}
