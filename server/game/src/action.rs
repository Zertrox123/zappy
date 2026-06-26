use crate::data::Resource;
use crate::data::{EntityId, Position};

#[derive(Clone, Debug, PartialEq)]
pub enum EAction {
    Forward,
    Right,
    Left,
    Look,
    Inventory,
    Fork,
    Eject,
    Broadcast(String),
    Take(Resource),
    Set(Resource),
    Incantation {
        position: Position,
        level: usize,
        participants: Vec<EntityId>,
    },
}

#[derive(Clone, Debug, PartialEq)]
pub struct Action {
    timeleft: usize,
    action: EAction,
}

impl Action {
    pub fn reduce_timeleft(&mut self) {
        if self.timeleft > 0 {
            self.timeleft -= 1;
        }
    }
    pub fn kind(&self) -> EAction {
        self.action.clone()
    }
    pub fn timeleft(&self) -> usize {
        self.timeleft
    }
    pub fn is_complete(&self) -> bool {
        self.timeleft == 0
    }

    pub fn new_forward() -> Self {
        Self {
            timeleft: 7,
            action: EAction::Forward,
        }
    }
    pub fn new_right() -> Self {
        Self {
            timeleft: 7,
            action: EAction::Right,
        }
    }

    pub fn new_left() -> Self {
        Self {
            timeleft: 7,
            action: EAction::Left,
        }
    }

    pub fn new_look() -> Self {
        Self {
            timeleft: 7,
            action: EAction::Look,
        }
    }
    pub fn new_inventory() -> Self {
        Self {
            timeleft: 1,
            action: EAction::Inventory,
        }
    }

    pub fn new_fork() -> Self {
        Self {
            timeleft: 42,
            action: EAction::Fork,
        }
    }

    pub fn new_eject() -> Self {
        Self {
            timeleft: 7,
            action: EAction::Eject,
        }
    }

    pub fn new_broadcast(text: String) -> Self {
        Self {
            timeleft: 7,
            action: EAction::Broadcast(text),
        }
    }

    pub fn new_take(resource: Resource) -> Self {
        Self {
            timeleft: 7,
            action: EAction::Take(resource),
        }
    }

    pub fn new_set(resource: Resource) -> Self {
        Self {
            timeleft: 7,
            action: EAction::Set(resource),
        }
    }

    pub fn new_incantation(position: Position, level: usize, participants: Vec<EntityId>) -> Self {
        Self {
            timeleft: 300,
            action: EAction::Incantation {
                position,
                level,
                participants,
            },
        }
    }
}
