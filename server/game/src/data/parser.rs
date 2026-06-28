use crate::action::Action;

use super::{Position, RESOURCES, Resource};

pub fn parse(buf: &str) -> Result<Action, String> {
    if buf.is_empty() {
        return Err("Empty packet".into());
    }

    match buf {
        "Forward" => Ok(Action::new_forward()),
        "Right" => Ok(Action::new_right()),
        "Left" => Ok(Action::new_left()),
        "Look" => Ok(Action::new_look()),
        "Inventory" => Ok(Action::new_inventory()),
        "Fork" => Ok(Action::new_fork()),
        "Eject" => Ok(Action::new_eject()),
        "Incantation" => Ok(Action::new_incantation(
            Position { x: 0, y: 0 },
            0,
            Vec::new(),
        )),
        _ if buf.starts_with("Broadcast ") && buf.len() > 10 => {
            Ok(Action::new_broadcast(buf[10..].to_string()))
        }
        _ if buf.starts_with("Take ") => parse_resource(&buf[5..]).map(Action::new_take),
        _ if buf.starts_with("Set ") => parse_resource(&buf[4..]).map(Action::new_set),
        _ => Err("KO".into()),
    }
}

fn parse_resource(name: &str) -> Result<Resource, String> {
    RESOURCES
        .into_iter()
        .find(|resource| resource.name() == name)
        .ok_or_else(|| "KO".into())
}
