use std::usize;

#[derive(Clone, Copy, Debug, PartialEq)]
pub enum EAction {
    Forward,
    Right,
    Left,
    Look,
    Inventoy,
    Fork,
    Eject,
    Broadcast,
}

#[derive(Clone, Copy, Debug, PartialEq)]
pub struct Action {
    timeleft: usize,
    pub action: EAction,
}

impl Action {
    pub fn reduce_timeleft(&mut self) {
        if self.timeleft > 0 {
            self.timeleft -= 1;
        }
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
            action: EAction::Inventoy,
        }
    }
}
