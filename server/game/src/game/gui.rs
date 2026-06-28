use super::{Direction, EntityId, Game, Position, SessionState};
use server::server::ClientReply;

impl Game {
    pub(super) fn send_to_guis(&mut self, response: &str) {
        let fds: Vec<_> = self
            .sessions
            .iter()
            .filter_map(|(fd, session)| {
                matches!(session.state, SessionState::Gui).then_some(*fd as i32)
            })
            .collect();

        for fd in fds {
            self.push_reply(fd, response);
        }
    }

    fn gui_direction(direction: Direction) -> usize {
        match direction {
            Direction::North => 1,
            Direction::Est => 2,
            Direction::South => 3,
            Direction::West => 4,
            Direction::None => 0,
        }
    }

    pub(super) fn gui_tile(&self, x: usize, y: usize) -> String {
        let mut counts = [0; 7];
        for resource in self.map.get(x as isize, y as isize).resources() {
            counts[*resource as usize] += 1;
        }
        format!(
            "bct {x} {y} {} {} {} {} {} {} {}\n",
            counts[0], counts[1], counts[2], counts[3], counts[4], counts[5], counts[6]
        )
    }

    pub(super) fn gui_tile_at(&self, position: Position) -> String {
        let (width, height) = self.map.dimensions();
        self.gui_tile(
            (position.x as isize).rem_euclid(width as isize) as usize,
            (position.y as isize).rem_euclid(height as isize) as usize,
        )
    }

    pub(super) fn gui_position_xy(&self, position: Position) -> (usize, usize) {
        let (width, height) = self.map.dimensions();
        (
            (position.x as isize).rem_euclid(width as isize) as usize,
            (position.y as isize).rem_euclid(height as isize) as usize,
        )
    }

    pub(super) fn gui_map(&self) -> String {
        let (width, height) = self.map.dimensions();
        let mut response = String::new();
        for y in 0..height {
            for x in 0..width {
                response.push_str(&self.gui_tile(x, y));
            }
        }
        response
    }

    pub(super) fn gui_player_line(&self, command: &str, player_id: EntityId) -> String {
        let player = &self.players[player_id as usize];
        let (x, y) = self.gui_position_xy(player.position());
        let inventory = player.inventory();
        match command {
            "pnw" => format!(
                "pnw #{} {} {} {} {} {}\n",
                player.get_id(),
                x,
                y,
                Self::gui_direction(player.direction()),
                player.level(),
                player.team()
            ),
            "ppo" => format!(
                "ppo #{} {} {} {}\n",
                player.get_id(),
                x,
                y,
                Self::gui_direction(player.direction())
            ),
            "plv" => format!("plv #{} {}\n", player.get_id(), player.level()),
            "pin" => format!(
                "pin #{} {} {} {} {} {} {} {} {} {}\n",
                player.get_id(),
                x,
                y,
                (player.get_saturation() / 126) + inventory[0],
                inventory[1],
                inventory[2],
                inventory[3],
                inventory[4],
                inventory[5],
                inventory[6]
            ),
            _ => String::new(),
        }
    }

    pub(super) fn gui_initial_state(&self) -> String {
        let (width, height) = self.map.dimensions();
        let mut response = format!("msz {width} {height}\n");
        response.push_str(&self.gui_map());
        for team in &self.teams {
            response.push_str(&format!("tna {team}\n"));
        }
        for player in &self.players {
            if player.raw_fd() >= 0 {
                response.push_str(&self.gui_player_line("pnw", player.get_id() as EntityId));
            }
        }
        for egg in &self.eggs {
            let (x, y) = self.gui_position_xy(egg.position);
            response.push_str(&format!("enw #{} #-1 {} {}\n", egg.id, x, y));
        }
        response.push_str(&format!("sgt {}\n", self.time_unit));
        response.push_str("smg GUI connected\n");
        response
    }

    pub(super) fn gui_command(&mut self, data: &str) -> ClientReply {
        let mut args = data.split_whitespace();
        match (args.next(), args.next(), args.next(), args.next()) {
            (Some("msz"), None, None, None) => {
                let (width, height) = self.map.dimensions();
                ClientReply::data(format!("msz {width} {height}\n").into_bytes())
            }
            (Some("mct"), None, None, None) => ClientReply::data(self.gui_map().into_bytes()),
            (Some("tna"), None, None, None) => ClientReply::data(
                self.teams
                    .iter()
                    .map(|team| format!("tna {team}\n"))
                    .collect::<String>()
                    .into_bytes(),
            ),
            (Some("bct"), Some(x), Some(y), None) => match (x.parse::<usize>(), y.parse::<usize>())
            {
                (Ok(x), Ok(y)) if x < self.map_dimensions().0 && y < self.map_dimensions().1 => {
                    ClientReply::data(self.gui_tile(x, y).into_bytes())
                }
                _ => ClientReply::data(b"sbp\n".to_vec()),
            },
            (Some(command @ ("ppo" | "plv" | "pin")), Some(id), None, None) => {
                self.gui_player_reply(id, command)
            }
            (Some("sgt"), None, None, None) => {
                ClientReply::data(format!("sgt {}\n", self.time_unit).into_bytes())
            }
            (Some("sst"), Some(value), None, None) => match value.parse::<usize>() {
                Ok(value) => {
                    self.time_unit = value;
                    ClientReply::data(format!("sst {}\n", self.time_unit).into_bytes())
                }
                Err(_) => ClientReply::data(b"sbp\n".to_vec()),
            },
            _ => ClientReply::data(b"suc\n".to_vec()),
        }
    }

    fn gui_player_reply(&self, id: &str, command: &str) -> ClientReply {
        let Some(id) = id.strip_prefix('#').and_then(|id| id.parse::<usize>().ok()) else {
            return ClientReply::data(b"sbp\n".to_vec());
        };
        if id >= self.players.len() {
            return ClientReply::data(b"sbp\n".to_vec());
        }
        ClientReply::data(self.gui_player_line(command, id as EntityId).into_bytes())
    }
}
