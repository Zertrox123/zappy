use server::server::ClientHandler;

use crate::{action::EAction, data::{Entity, EntityId, Map, Resource}};

const REFILL_INTERVAL: u64 = 20;

pub struct Game {
    map: Map,
    players: Vec<Entity>,
    teams: Vec<String>,
    clients_per_team: usize,
    ticks: u64,
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

    pub fn do_action(&mut self) {
        for player in &mut self.players {
            if player.actions.len() < 1 {
                continue;
            }
            for action in &mut player.actions {
                action.reduce_timeleft();
                println!("{}", action.timeleft())
            }
            let done = player.actions.iter().position(|a| a.is_complete());
            if let Some(i) = done {
                let action = player.actions.remove(i);
                match action.action {
                    EAction::Forward => player.forward(),
                    EAction::Left => player.rotate(crate::data::Rotation::Left),
                    EAction::Right => player.rotate(crate::data::Rotation::Right),
                    _ => {}
                }
            }
        }
    }

    pub fn run_ticks(&mut self) {
        self.ticks += 1;
        if self.ticks % REFILL_INTERVAL == 0 {
            self.map.refill();
        }
        self.do_action();
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
