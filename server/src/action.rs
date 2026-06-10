#[derive(Clone, Copy, Debug, PartialEq)]
pub enum eAction {
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
    action: eAction,
}

impl Action {
    pub fn new_forward() -> Self {
        Self {
            timeleft: 7,
            action: eAction::Forward,
        }
    }
    pub fn new_right() -> Self {
        Self {
            timeleft: 7,
            action: eAction::Right,
        }
    }

    pub fn new_left() -> Self {
        Self {
            timeleft: 7,
            action: eAction::Left,
        }
    }

    pub fn new_look() -> Self {
        Self {
            timeleft: 7,
            action: eAction::Look,
        }
    }
    pub fn new_inventory() -> Self {
        Self {
            timeleft: 1,
            action: eAction::Inventoy,
        }
    }
}
