use rand::Rng;

use crate::action::Action;

pub type EntityId = u8;

#[derive(Clone, Debug)]
pub struct Map {
    tiles: Vec<Vec<Tile>>,
    height: usize,
    width: usize,
}

impl Map {
    pub fn new(width: usize, height: usize) -> Self {
        let mut map: Vec<Vec<Tile>> = Vec::with_capacity(height);

        for i in 0..height {
            map.push(Vec::with_capacity(width));
            for _ in 0..width {
                map.get_mut(i).unwrap().push(Tile::new_empty());
            }
        }

        Map {
            tiles: map,
            width,
            height,
        }
    }

    pub fn dimensions(&self) -> (usize, usize) {
        (self.width, self.height)
    }

    pub fn get(&self, x: isize, y: isize) -> &Tile {
        let wisize = self.width as isize;
        let hisize = self.height as isize;

        let x: usize = x.rem_euclid(wisize) as usize;
        let y: usize = y.rem_euclid(hisize) as usize;

        self.tiles.get(y).unwrap().get(x).unwrap()
    }
    pub fn get_mut(&mut self, x: isize, y: isize) -> &mut Tile {
        let wisize = self.width as isize;
        let hisize = self.height as isize;

        let x: usize = x.rem_euclid(wisize) as usize;
        let y: usize = y.rem_euclid(hisize) as usize;

        self.tiles.get_mut(y).unwrap().get_mut(x).unwrap()
    }
    pub fn populate(&mut self) {
        self.refill();
    }

    pub fn count(&self, resource: Resource) -> usize {
        let mut total = 0;
        for row in &self.tiles {
            for tile in row {
                total += tile
                    .stone
                    .iter()
                    .filter(|stone| **stone == resource)
                    .count();
            }
        }
        total
    }

    pub fn max_resources(&self, resource: Resource) -> usize {
        max_for(self.width * self.height, resource)
    }

    pub fn refill(&mut self) {
        for resource in RESOURCES {
            let current = self.count(resource);
            let max = self.max_resources(resource);
            if current < max {
                self.spawn(resource, max - current);
            }
        }
    }

    pub fn deplete(&mut self, resource: Resource, amount: usize) -> usize {
        let mut removed = 0;
        for row in &mut self.tiles {
            for tile in row {
                tile.stone.retain(|stone| {
                    if removed < amount && *stone == resource {
                        removed += 1;
                        false
                    } else {
                        true
                    }
                });
            }
        }
        removed
    }

    fn spawn(&mut self, resource: Resource, amount: usize) {
        for _ in 0..amount {
            let x = rand::thread_rng().gen_range(0..self.width);
            let y = rand::thread_rng().gen_range(0..self.height);
            self.tiles[y][x].stone.push(resource);
        }
    }

    pub fn show_map(&mut self) {
        for i in 0..self.height {
            for y in 0..self.width {
                let tile: &mut Tile = self.tiles.get_mut(i).unwrap().get_mut(y).unwrap();
                print!("{:?}\t\t", tile);
            }
            println!();
        }
    }
}

#[derive(Clone, Debug)]
pub struct Tile {
    stone: Vec<Resource>,
    entity: Option<Entity>,
}

impl Tile {
    pub fn new_empty() -> Self {
        Tile {
            stone: Vec::new(),
            entity: None,
        }
    }
    pub fn get_value(&self) -> usize {
        let mut total = 0;
        if !self.stone.is_empty() {
            let mut a = 10;
            a += match self.stone[0] {
                Resource::Food => 1,
                Resource::Sibur => 2,
                Resource::Phiras => 3,
                Resource::Linemate => 4,
                Resource::Mendiane => 5,
                Resource::Thystame => 6,
                Resource::Deraumere => 7,
            };
            total += a;
        }
        if self.entity.is_some() {
            total += 200 * self.entity.as_ref().unwrap().get_id();
        }
        total
    }

    pub fn eq(&self, _rhs: Tile) -> bool {
        _rhs.get_value() == self.get_value()
    }
}

#[derive(Clone, Copy, PartialEq, Debug)]
pub struct Position {
    pub x: i8,
    pub y: i8,
}

impl Position {
    pub fn set(&mut self, x: i8, y: i8) {
        self.y = y;
        self.x = x;
    }
}

#[derive(Clone, PartialEq, Debug)]
pub struct Entity {
    id: usize,
    team: String,
    saturation: usize,
    level: usize,
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
            team: String::from(""),
            saturation: 142,
            level: 0,
            dummy: true,
            pos: Position { x: 0, y: 0 },
            direction: Direction::South,
            actions: Vec::new(),
            reply: String::new(),
        }
    }

    pub fn set_reply(&mut self, msg: String) {
        self.reply = msg;
    }

    pub fn get_reply(&mut self) -> &String {
        &self.reply
    }

    pub fn get_id(&self) -> usize {
        self.id
    }
    pub fn set_id(&mut self, id: usize) {
        self.id = id;
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
}

#[derive(Clone, Copy, Debug, PartialEq)]
pub enum PacketDirection {
    Incoming,
    Outgoing,
}

#[derive(Clone, Copy, PartialEq, Debug)]
pub enum Direction {
    Est,
    South,
    North,
    West,
    None,
}

#[derive(Clone, Copy, PartialEq, Debug)]
pub enum Rotation {
    Right,
    Left,
    None,
}

pub fn parse(buf: &str) -> Result<Action, String> {
    if buf.is_empty() {
        return Err("Empty packet".into());
    }

    match buf.split(' ').next().unwrap() {
        "Forward" => Ok(Action::new_forward()),
        "Right" => Ok(Action::new_right()),
        "Left" => Ok(Action::new_left()),
        "Look" => Ok(Action::new_look()),
        "Inventory" => Ok(Action::new_inventory()),
        _ => Err("KO".into()),
    }
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum Resource {
    Food = 0,
    Linemate,
    Deraumere,
    Sibur,
    Mendiane,
    Phiras,
    Thystame,
}

pub(crate) const RESOURCES: [Resource; 7] = [
    Resource::Food,
    Resource::Linemate,
    Resource::Deraumere,
    Resource::Sibur,
    Resource::Mendiane,
    Resource::Phiras,
    Resource::Thystame,
];

fn max_for(area: usize, resource: Resource) -> usize {
    (area as f32 * resource.get_density()) as usize
}

impl Resource {
    fn get_density(&self) -> f32 {
        match self {
            Resource::Food => 0.50,
            Resource::Linemate => 0.30,
            Resource::Deraumere => 0.15,
            Resource::Sibur => 0.10,
            Resource::Mendiane => 0.10,
            Resource::Phiras => 0.08,
            Resource::Thystame => 0.05,
        }
    }
}
