use std::ops;

use crate::action::Action;

type EntityId = u8;


#[derive(Clone, Debug)]
pub struct Map {
    Tiles: Vec<Vec<Tile>>,
    height: usize,
    width: usize,
}

impl Map {
    pub fn new(width: usize, height: usize) -> Self {
        let mut map: Vec<Vec<Tile>> = Vec::with_capacity(height);

        for i in 0..height {
            map.push(Vec::with_capacity(width));
            for _ in 0..width {
                map.get_mut(i).unwrap().push(Tile::Empty);
            }
        }

        Map {
            Tiles: map,
            width,
            height,
        }
    }

    pub fn show_map(&mut self) {
        for i in 0..self.height {
            for y in 0..self.width {
                let tile: &mut Tile = self.Tiles.get_mut(i).unwrap().get_mut(y).unwrap();
                print!("{} ", tile.get_char());
            }
            println!();
        }
    }
}

#[derive(Clone, Debug)]
pub enum Tile {
    Stone(Option<Resource>),
    Entity(Option<Entity>),
    Empty,
}

impl Tile {
    pub fn get_value(&self) -> usize {
        match self {
            Tile::Empty => 0,
            Tile::Stone(ing) => {
                let mut a = 10;
                a += match ing.unwrap() {
                    Resource::Food => 1,
                    Resource::Sibur => 2,
                    Resource::Phiras => 3,
                    Resource::Linemate => 4,
                    Resource::Mendiane => 5,
                    Resource::Thystame => 6,
                    Resource::Deraumere => 7,
                    _ => 0,
                };
                a
            }
            Tile::Entity(entity) => 200 * entity.as_ref().unwrap().getId(),
        }
    }

    pub fn get_char(&self) -> char {
        match self {
            Tile::Empty => '0',
            Tile::Stone(ing) => match ing.unwrap() {
                Resource::Food => '1',
                Resource::Sibur => '2',
                Resource::Phiras => '3',
                Resource::Linemate => '4',
                Resource::Mendiane => '5',
                Resource::Thystame => '6',
                Resource::Deraumere => '7',
                _ => '0',
            },
            Tile::Entity(entity) => 'e',
        }
    }

    pub fn eq(&self, _rhs: Tile) -> bool {
        _rhs.get_value() == self.get_value()
    }
}

#[derive(Clone, Copy, PartialEq, Debug)]
pub struct Position {
    pub x: u8,
    pub y: u8,
}

impl Position {
    pub fn set(&mut self, x: u8, y: u8) {
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
    pub actions: Vec<Action>,
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
            actions: Vec::new(),
        }
    }
    pub fn getId(&self) -> usize {
        return self.id;
    }
    pub fn setId(&mut self, id: usize) {
        self.id = id;
    }
    pub fn add_action(&mut self, action: Action) {
        self.actions.push(action);
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

pub fn parse(buf: &str) -> Result<Action, String> {
    if buf.is_empty() {
        return Err("Empty packet".into());
    }
    println!("{}", buf);
    println!("{:#?}", buf.split(' '));
    match buf.split(' ').next().unwrap() {
        "Forward" => Ok(Action::new_forward()),
        "Right" => Ok(Action::new_right()),
        "Left" => Ok(Action::new_left()),
        "Look" => Ok(Action::new_look()),
        "Inventory" => Ok(Action::new_inventory()),
        //"Broadcast" => return Ok(Action::new()),
        _ => Err("KO".into()),
    }
}

#[derive(Clone, Copy, Debug)]
#[allow(unused_variables)]
#[allow(dead_code)]
enum Resource {
    Food = 0,
    Linemate,
    Deraumere,
    Sibur,
    Mendiane,
    Phiras,
    Thystame,
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
