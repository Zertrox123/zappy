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

pub(super) fn max_for(area: usize, resource: Resource) -> usize {
    ((area as f32 * resource.get_density()) as usize).max(1)
}

impl Resource {
    pub fn name(self) -> &'static str {
        match self {
            Resource::Food => "food",
            Resource::Linemate => "linemate",
            Resource::Deraumere => "deraumere",
            Resource::Sibur => "sibur",
            Resource::Mendiane => "mendiane",
            Resource::Phiras => "phiras",
            Resource::Thystame => "thystame",
        }
    }

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
