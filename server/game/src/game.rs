use crate::action::EAction;
use crate::data::{Direction, Entity, EntityId, Map, Position, RESOURCES, Resource, Rotation};
use std::collections::HashMap;
use std::usize;

mod gui;
mod incantation;
mod session;

#[cfg(test)]
mod action_tests;

const REFILL_INTERVAL: u64 = 20;
const MAX_ACTION: usize = 10;
const WIN_LEVEL: usize = 8;
const WIN_PLAYERS: usize = 6;

enum SessionState {
    AwaitingTeamName,
    Ready { player_id: EntityId, team: String },
    Gui,
}

struct Session {
    state: SessionState,
}

struct Egg {
    id: usize,
    team: String,
    position: Position,
}

pub struct Game {
    map: Map,
    players: Vec<Entity>,
    teams: Vec<String>,
    clients_per_team: usize,
    time_unit: usize,
    team_capacity: HashMap<String, usize>,
    eggs: Vec<Egg>,
    next_egg_id: usize,
    ticks: u64,
    sessions: HashMap<u64, Session>,
    reply: HashMap<i32, String>,
    frozen: Vec<EntityId>,
}

impl Game {
    pub fn new(width: usize, height: usize, teams: Vec<String>, clients_per_team: usize) -> Self {
        Self::new_with_frequency(width, height, teams, clients_per_team, 100)
    }

    pub fn new_with_frequency(
        width: usize,
        height: usize,
        teams: Vec<String>,
        clients_per_team: usize,
        time_unit: usize,
    ) -> Self {
        let mut map = Map::new(width, height);
        map.populate();
        let team_capacity = teams
            .iter()
            .map(|team| (team.clone(), clients_per_team))
            .collect();
        Game {
            map,
            players: Vec::new(),
            teams,
            clients_per_team,
            time_unit,
            team_capacity,
            eggs: Vec::new(),
            next_egg_id: 0,
            ticks: 0,
            sessions: HashMap::new(),
            reply: HashMap::new(),
            frozen: Vec::new(),
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
        for player_index in 0..self.players.len() {
            let completed_action = {
                let player = &mut self.players[player_index];
                let Some(action) = player.actions.first_mut() else {
                    continue;
                };

                action.reduce_timeleft();
                action
                    .is_complete()
                    .then(|| player.actions.remove(0).kind())
            };

            let Some(action) = completed_action else {
                continue;
            };

            println!(
                "[GAME] player id: {} finished {:?}",
                self.players[player_index].get_id(),
                action
            );
            let response = match action {
                EAction::Forward => {
                    self.players[player_index].forward();
                    let msg = self
                        .gui_player_line("ppo", self.players[player_index].get_id() as EntityId);
                    self.send_to_guis(&msg);
                    "ok\n".to_string()
                }
                EAction::Left => {
                    self.players[player_index].rotate(Rotation::Left);
                    let msg = self
                        .gui_player_line("ppo", self.players[player_index].get_id() as EntityId);
                    self.send_to_guis(&msg);
                    "ok\n".to_string()
                }
                EAction::Right => {
                    self.players[player_index].rotate(Rotation::Right);
                    let msg = self
                        .gui_player_line("ppo", self.players[player_index].get_id() as EntityId);
                    self.send_to_guis(&msg);
                    "ok\n".to_string()
                }
                EAction::Look => self.look(player_index),
                EAction::Inventory => self.inventory(player_index),
                EAction::Take(resource) => self.take(player_index, resource),
                EAction::Set(resource) => self.set(player_index, resource),
                EAction::Broadcast(text) => {
                    self.broadcast(player_index, &text);
                    self.send_to_guis(&format!(
                        "pbc #{} {}\n",
                        self.players[player_index].get_id(),
                        text
                    ));
                    "ok\n".to_string()
                }
                EAction::Eject => self.eject(player_index),
                EAction::Fork => {
                    let team = self.players[player_index].team().to_string();
                    let position = self.players[player_index].position();
                    let player_id = self.players[player_index].get_id();
                    let egg_id = self.next_egg_id;
                    self.next_egg_id += 1;
                    self.eggs.push(Egg {
                        id: egg_id,
                        team: team.clone(),
                        position,
                    });
                    *self.team_capacity.get_mut(&team).unwrap() += 1;
                    let (x, y) = self.gui_position_xy(position);
                    self.send_to_guis(&format!(
                        "pfk #{}\nenw #{} #{} {} {}\n",
                        player_id, egg_id, player_id, x, y
                    ));
                    "ok\n".to_string()
                }
                EAction::Incantation {
                    position,
                    level,
                    participants,
                } => self.finish_incantation(position, level, &participants),
            };
            let fd = self.players[player_index].raw_fd();
            self.push_reply(fd, &response);
        }
    }

    fn push_reply(&mut self, fd: i32, response: &str) {
        self.reply.entry(fd).or_default().push_str(response);
    }

    fn inventory(&self, player_index: usize) -> String {
        let player = &self.players[player_index];
        let inventory = player.inventory();
        format!(
            "[{}]\n",
            RESOURCES
                .iter()
                .map(|resource| {
                    let amount = if *resource == Resource::Food {
                        (player.get_saturation() / 126) + inventory[*resource as usize]
                    } else {
                        inventory[*resource as usize]
                    };
                    format!("{} {}", resource.name(), amount)
                })
                .collect::<Vec<_>>()
                .join(", ")
        )
    }

    fn take(&mut self, player_index: usize, resource: Resource) -> String {
        let position = self.players[player_index].position();
        if !self.map.take(position, resource) {
            return "ko\n".into();
        }
        self.players[player_index].take(resource);
        self.send_to_guis(&format!(
            "pgt #{} {}\n{}",
            self.players[player_index].get_id(),
            resource as usize,
            self.gui_tile_at(position)
        ));
        "ok\n".into()
    }

    fn set(&mut self, player_index: usize, resource: Resource) -> String {
        if !self.players[player_index].set(resource) {
            return "ko\n".into();
        }
        let position = self.players[player_index].position();
        self.map.put(position, resource);
        self.send_to_guis(&format!(
            "pdr #{} {}\n{}",
            self.players[player_index].get_id(),
            resource as usize,
            self.gui_tile_at(position)
        ));
        "ok\n".into()
    }

    fn shortest_delta(from: i8, to: i8, size: usize) -> isize {
        let delta = (to as isize - from as isize).rem_euclid(size as isize);
        if delta > size as isize / 2 {
            delta - size as isize
        } else {
            delta
        }
    }

    fn sound_tile(&self, receiver: usize, source: Position) -> usize {
        let player = &self.players[receiver];
        let position = player.position();
        let (width, height) = self.map.dimensions();
        let dx = Self::shortest_delta(position.x, source.x, width).signum();
        let dy = Self::shortest_delta(position.y, source.y, height).signum();
        if dx == 0 && dy == 0 {
            return 0;
        }
        let absolute = match (dx, dy) {
            (0, 1) => 1,
            (-1, 1) => 2,
            (-1, 0) => 3,
            (-1, -1) => 4,
            (0, -1) => 5,
            (1, -1) => 6,
            (1, 0) => 7,
            (1, 1) => 8,
            _ => unreachable!(),
        };
        let rotation = match player.direction() {
            Direction::North => 0,
            Direction::West => 2,
            Direction::South => 4,
            Direction::Est => 6,
            Direction::None => 0,
        };
        (absolute + 7 - rotation) % 8 + 1
    }

    fn same_tile(&self, first: Position, second: Position) -> bool {
        let (width, height) = self.map.dimensions();
        (first.x as isize).rem_euclid(width as isize)
            == (second.x as isize).rem_euclid(width as isize)
            && (first.y as isize).rem_euclid(height as isize)
                == (second.y as isize).rem_euclid(height as isize)
    }

    fn broadcast(&mut self, source: usize, text: &str) {
        let position = self.players[source].position();
        for receiver in 0..self.players.len() {
            let fd = self.players[receiver].raw_fd();
            if receiver != source && fd >= 0 {
                self.push_reply(
                    fd,
                    &format!(
                        "message {}, {}\n",
                        self.sound_tile(receiver, position),
                        text
                    ),
                );
            }
        }
    }

    fn eject(&mut self, source: usize) -> String {
        let origin = self.players[source].position();
        let direction = self.players[source].direction();
        let victims: Vec<_> = (0..self.players.len())
            .filter(|&index| {
                index != source
                    && self.players[index].raw_fd() >= 0
                    && self.same_tile(self.players[index].position(), origin)
            })
            .collect();
        let ejected = !victims.is_empty();
        for victim in victims {
            let mut destination = origin;
            match direction {
                Direction::North => destination.y += 1,
                Direction::South => destination.y -= 1,
                Direction::Est => destination.x += 1,
                Direction::West => destination.x -= 1,
                Direction::None => {}
            }
            self.players[victim].set_position(destination);
            let tile = self.sound_tile(victim, origin);
            self.push_reply(self.players[victim].raw_fd(), &format!("eject: {tile}\n"));
            let msg = self.gui_player_line("ppo", self.players[victim].get_id() as EntityId);
            self.send_to_guis(&msg);
        }

        let mut destroyed = Vec::new();
        let (width, height) = self.map.dimensions();
        self.eggs.retain(|egg| {
            if (egg.position.x as isize).rem_euclid(width as isize)
                == (origin.x as isize).rem_euclid(width as isize)
                && (egg.position.y as isize).rem_euclid(height as isize)
                    == (origin.y as isize).rem_euclid(height as isize)
            {
                destroyed.push((egg.id, egg.team.clone()));
                false
            } else {
                true
            }
        });
        for (egg_id, team) in destroyed {
            *self.team_capacity.get_mut(&team).unwrap() -= 1;
            self.send_to_guis(&format!("edi #{egg_id}\n"));
        }
        if ejected {
            self.send_to_guis(&format!("pex #{}\n", self.players[source].get_id()));
            "ok\n".into()
        } else {
            "ko\n".into()
        }
    }

    fn vision_vectors(direction: Direction) -> ((isize, isize), (isize, isize)) {
        match direction {
            Direction::North => ((0, 1), (1, 0)),
            Direction::South => ((0, -1), (-1, 0)),
            Direction::Est => ((1, 0), (0, -1)),
            Direction::West => ((-1, 0), (0, 1)),
            Direction::None => ((0, 0), (0, 0)),
        }
    }

    fn tile_content(&self, x: isize, y: isize) -> String {
        let (width, height) = self.map.dimensions();
        let wrapped_x = x.rem_euclid(width as isize);
        let wrapped_y = y.rem_euclid(height as isize);
        let mut objects = Vec::new();

        for player in &self.players {
            let position = player.position();
            if (position.x as isize).rem_euclid(width as isize) == wrapped_x
                && (position.y as isize).rem_euclid(height as isize) == wrapped_y
            {
                objects.push("player");
            }
        }
        for resource in self.map.get(wrapped_x, wrapped_y).resources() {
            objects.push(resource.name());
        }

        objects.join(" ")
    }

    fn look(&self, player_index: usize) -> String {
        let player = &self.players[player_index];
        let position = player.position();
        let (forward, right) = Self::vision_vectors(player.direction());
        let mut tiles = Vec::new();

        for depth in 0..=player.level() as isize {
            for lateral in -depth..=depth {
                let x = position.x as isize + forward.0 * depth + right.0 * lateral;
                let y = position.y as isize + forward.1 * depth + right.1 * lateral;
                tiles.push(self.tile_content(x, y));
            }
        }

        format!("[{}]\n", tiles.join(","))
    }

    pub fn run_ticks(&mut self) {
        self.ticks += 1;
        if self.ticks % REFILL_INTERVAL == 0 {
            for position in self.map.refill() {
                let tile = self.gui_tile_at(position);
                self.send_to_guis(&tile);
            }
        }
        let mut dead_players = Vec::new();
        for player_index in 0..self.players.len() {
            let death = {
                let player = &mut self.players[player_index];
                if !player.is_alive() {
                    continue;
                }
                if player.get_saturation() == 0 {
                    if player.inventory()[Resource::Food as usize] == 0 {
                        let fd = player.raw_fd();
                        let player_id = player.get_id();
                        player.set_alive(false);
                        player.set_raw_fd(-1);
                        Some((fd, player_id))
                    } else {
                        player.inventory_mut()[Resource::Food as usize] -= 1;
                        player.set_saturation(126);
                        None
                    }
                } else {
                    player.set_saturation(player.get_saturation() - 1);
                    None
                }
            };
            if let Some((fd, player_id)) = death {
                self.push_reply(fd, "dead\n");
                self.send_to_guis(&format!("pdi #{player_id}\n"));
                dead_players.push(player_index);
            }
        }
        for player_index in dead_players.into_iter().rev() {
            self.remove_player(player_index);
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
}
