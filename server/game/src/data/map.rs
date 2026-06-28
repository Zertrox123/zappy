use super::resource::max_for;
use super::{Entity, RESOURCES, Resource};

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
        let x: usize = x.rem_euclid(self.width as isize) as usize;
        let y: usize = y.rem_euclid(self.height as isize) as usize;
        self.tiles.get(y).unwrap().get(x).unwrap()
    }

    pub fn get_mut(&mut self, x: isize, y: isize) -> &mut Tile {
        let x: usize = x.rem_euclid(self.width as isize) as usize;
        let y: usize = y.rem_euclid(self.height as isize) as usize;
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

    pub fn refill(&mut self) -> Vec<Position> {
        let mut positions = Vec::new();
        for resource in RESOURCES {
            let current = self.count(resource);
            let max = self.max_resources(resource);
            if current < max {
                positions.extend(self.spawn(resource, max - current));
            }
        }
        positions
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

    pub fn take(&mut self, position: Position, resource: Resource) -> bool {
        let stones = &mut self.get_mut(position.x as isize, position.y as isize).stone;
        let Some(index) = stones.iter().position(|item| *item == resource) else {
            return false;
        };
        stones.remove(index);
        true
    }

    pub fn put(&mut self, position: Position, resource: Resource) {
        self.get_mut(position.x as isize, position.y as isize)
            .stone
            .push(resource);
    }

    fn spawn(&mut self, resource: Resource, amount: usize) -> Vec<Position> {
        let mut positions = Vec::new();
        for _ in 0..amount {
            let mut best_x = 0;
            let mut best_y = 0;
            let mut best_count = usize::MAX;

            for y in 0..self.height {
                for x in 0..self.width {
                    let count = self.tiles[y][x].stone.len();
                    if count < best_count {
                        best_x = x;
                        best_y = y;
                        best_count = count;
                    }
                }
            }

            self.tiles[best_y][best_x].stone.push(resource);
            positions.push(Position {
                x: best_x as i8,
                y: best_y as i8,
            });
        }
        positions
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

    pub fn resources(&self) -> &[Resource] {
        &self.stone
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
