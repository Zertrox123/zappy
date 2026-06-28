use std::os::fd::RawFd;

use crate::action::Action;

use super::{Direction, Position, Resource, Rotation};

pub type EntityId = u8;

#[derive(Clone, PartialEq, Debug)]
pub struct Entity {
    id: usize,
    raw_fd: RawFd,
    team: String,
    saturation: usize,
    inventory: [usize; 7],
    level: usize,
    alive: bool,
    dummy: bool,
    pos: Position,
    direction: Direction,
    pub actions: Vec<Action>,
    reply: String,
}

impl Entity {
    pub fn new_dummy() -> Self {
        Entity {
            id: 0,
            raw_fd: -1,
            team: String::from(""),
            saturation: 1260,
            inventory: [0, 0, 0, 0, 0, 0, 0],
            level: 1,
            dummy: true,
            alive: true,
            pos: Position { x: 0, y: 0 },
            direction: Direction::South,
            actions: Vec::new(),
            reply: String::new(),
        }
    }

    pub fn set_reply(&mut self, msg: String) {
        self.reply = msg;
    }

    pub fn is_alive(&self) -> bool {
        self.alive
    }

    pub fn set_alive(&mut self, value: bool) {
        self.alive = value;
    }

    pub fn get_reply(&mut self, _msg: String) -> String {
        self.reply.clone()
    }

    pub fn get_id(&self) -> usize {
        self.id
    }

    pub fn set_id(&mut self, id: usize) {
        self.id = id;
    }

    pub fn raw_fd(&self) -> RawFd {
        self.raw_fd
    }

    pub fn set_raw_fd(&mut self, raw_fd: RawFd) {
        self.raw_fd = raw_fd;
    }

    pub fn position(&self) -> Position {
        self.pos
    }

    pub fn direction(&self) -> Direction {
        self.direction
    }

    pub fn level(&self) -> usize {
        self.level
    }

    pub fn level_up(&mut self) {
        self.level += 1;
    }

    pub fn set_position(&mut self, position: Position) {
        self.pos = position;
    }

    pub fn inventory(&self) -> &[usize; 7] {
        &self.inventory
    }

    pub fn inventory_mut(&mut self) -> &mut [usize; 7] {
        &mut self.inventory
    }

    pub fn set_saturation(&mut self, value: usize) {
        self.saturation = value;
    }

    pub fn get_saturation(&self) -> usize {
        self.saturation
    }

    pub fn take(&mut self, resource: Resource) {
        self.inventory[resource as usize] += 1;
    }

    pub fn set(&mut self, resource: Resource) -> bool {
        let amount = &mut self.inventory[resource as usize];
        if *amount == 0 {
            return false;
        }
        *amount -= 1;
        true
    }

    pub fn forward(&mut self) {
        match self.direction {
            Direction::North => self.pos.y += 1,
            Direction::South => self.pos.y -= 1,
            Direction::Est => self.pos.x += 1,
            Direction::West => self.pos.x -= 1,
            Direction::None => {}
        }
    }

    pub fn rotate(&mut self, direction: Rotation) {
        self.direction = match (self.direction, direction) {
            (_, Rotation::None) => self.direction,
            (Direction::North, Rotation::Right) => Direction::Est,
            (Direction::Est, Rotation::Right) => Direction::South,
            (Direction::South, Rotation::Right) => Direction::West,
            (Direction::West, Rotation::Right) => Direction::North,
            (Direction::North, Rotation::Left) => Direction::West,
            (Direction::West, Rotation::Left) => Direction::South,
            (Direction::South, Rotation::Left) => Direction::Est,
            (Direction::Est, Rotation::Left) => Direction::North,
            (_, _) => self.direction,
        }
    }

    pub fn add_action(&mut self, action: Action) -> bool {
        if self.actions.len() >= 10 {
            return false;
        }
        self.actions.push(action);
        true
    }

    pub fn set_team(&mut self, name: &String) {
        self.team = name.clone();
    }

    pub fn team(&self) -> &str {
        &self.team
    }
}
