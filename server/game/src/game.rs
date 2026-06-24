use crate::action::EAction;
use crate::data::{
    Direction, Entity, EntityId, Map, Position, RESOURCES, Resource, Rotation, parse,
};
use server::server::{ClientHandler, ClientReply};
use std::collections::HashMap;
use std::usize;

const REFILL_INTERVAL: u64 = 20;
const MAX_ACTION: usize = 10;

enum SessionState {
    AwaitingTeamName,
    Ready { player_id: EntityId, team: String },
}

struct Session {
    state: SessionState,
}

struct Egg {
    team: String,
    position: Position,
}

pub struct Game {
    map: Map,
    players: Vec<Entity>,
    teams: Vec<String>,
    clients_per_team: usize,
    team_capacity: HashMap<String, usize>,
    eggs: Vec<Egg>,
    ticks: u64,
    sessions: HashMap<u64, Session>,
    reply: HashMap<i32, String>,
}

impl Game {
    pub fn new(width: usize, height: usize, teams: Vec<String>, clients_per_team: usize) -> Self {
        let mut map = Map::new(width, height);
        map.populate();
        let team_capacity = teams
            .iter()
            .map(|team| (team.clone(), clients_per_team))
            .collect();
        Game {
            map,
            players: Vec::new(),
            teams,
            clients_per_team,
            team_capacity,
            eggs: Vec::new(),
            ticks: 0,
            sessions: HashMap::new(),
            reply: HashMap::new(),
        }
    }

    pub fn map_dimensions(&self) -> (usize, usize) {
        self.map.dimensions()
    }

    pub fn teams(&self) -> &[String] {
        &self.teams
    }

    pub fn clients_per_team(&self) -> usize {
        self.clients_per_team
    }

    pub fn do_action(&mut self) {
        for player_index in 0..self.players.len() {
            let completed_action = {
                let player = &mut self.players[player_index];
                let Some(action) = player.actions.first_mut() else {
                    continue;
                };

                action.reduce_timeleft();
                action
                    .is_complete()
                    .then(|| player.actions.remove(0).kind())
            };

            let Some(action) = completed_action else {
                continue;
            };

            println!(
                "[GAME] player id: {} finished {:?}",
                self.players[player_index].get_id(),
                action
            );
            let response = match action {
                EAction::Forward => {
                    self.players[player_index].forward();
                    "ok\n".to_string()
                }
                EAction::Left => {
                    self.players[player_index].rotate(Rotation::Left);
                    "ok\n".to_string()
                }
                EAction::Right => {
                    self.players[player_index].rotate(Rotation::Right);
                    "ok\n".to_string()
                }
                EAction::Look => self.look(player_index),
                EAction::Inventory => self.inventory(player_index),
                EAction::Take(resource) => self.take(player_index, resource),
                EAction::Set(resource) => self.set(player_index, resource),
                EAction::Broadcast(text) => {
                    self.broadcast(player_index, &text);
                    "ok\n".to_string()
                }
                EAction::Eject => self.eject(player_index),
                EAction::Fork => {
                    let player = &self.players[player_index];
                    self.eggs.push(Egg {
                        team: player.team().to_string(),
                        position: player.position(),
                    });
                    *self.team_capacity.get_mut(player.team()).unwrap() += 1;
                    "ok\n".to_string()
                }
            };
            let fd = self.players[player_index].raw_fd();
            self.push_reply(fd, &response);
        }
    }

    fn push_reply(&mut self, fd: i32, response: &str) {
        self.reply.entry(fd).or_default().push_str(response);
    }

    fn inventory(&self, player_index: usize) -> String {
        let inventory = self.players[player_index].inventory();
        format!(
            "[{}]\n",
            RESOURCES
                .iter()
                .map(|resource| format!("{} {}", resource.name(), inventory[*resource as usize]))
                .collect::<Vec<_>>()
                .join(", ")
        )
    }

    fn take(&mut self, player_index: usize, resource: Resource) -> String {
        let position = self.players[player_index].position();
        if !self.map.take(position, resource) {
            return "ko\n".into();
        }
        self.players[player_index].take(resource);
        "ok\n".into()
    }

    fn set(&mut self, player_index: usize, resource: Resource) -> String {
        if !self.players[player_index].set(resource) {
            return "ko\n".into();
        }
        self.map
            .put(self.players[player_index].position(), resource);
        "ok\n".into()
    }

    fn shortest_delta(from: i8, to: i8, size: usize) -> isize {
        let delta = (to as isize - from as isize).rem_euclid(size as isize);
        if delta > size as isize / 2 {
            delta - size as isize
        } else {
            delta
        }
    }

    fn sound_tile(&self, receiver: usize, source: Position) -> usize {
        let player = &self.players[receiver];
        let position = player.position();
        let (width, height) = self.map.dimensions();
        let dx = Self::shortest_delta(position.x, source.x, width).signum();
        let dy = Self::shortest_delta(position.y, source.y, height).signum();
        if dx == 0 && dy == 0 {
            return 0;
        }
        let absolute = match (dx, dy) {
            (0, 1) => 1,
            (-1, 1) => 2,
            (-1, 0) => 3,
            (-1, -1) => 4,
            (0, -1) => 5,
            (1, -1) => 6,
            (1, 0) => 7,
            (1, 1) => 8,
            _ => unreachable!(),
        };
        let rotation = match player.direction() {
            Direction::North => 0,
            Direction::West => 2,
            Direction::South => 4,
            Direction::Est => 6,
            Direction::None => 0,
        };
        (absolute + 7 - rotation) % 8 + 1
    }

    fn same_tile(&self, first: Position, second: Position) -> bool {
        let (width, height) = self.map.dimensions();
        (first.x as isize).rem_euclid(width as isize)
            == (second.x as isize).rem_euclid(width as isize)
            && (first.y as isize).rem_euclid(height as isize)
                == (second.y as isize).rem_euclid(height as isize)
    }

    fn broadcast(&mut self, source: usize, text: &str) {
        let position = self.players[source].position();
        for receiver in 0..self.players.len() {
            let fd = self.players[receiver].raw_fd();
            if receiver != source && fd >= 0 {
                self.push_reply(
                    fd,
                    &format!(
                        "message {}, {}\n",
                        self.sound_tile(receiver, position),
                        text
                    ),
                );
            }
        }
    }

    fn eject(&mut self, source: usize) -> String {
        let origin = self.players[source].position();
        let direction = self.players[source].direction();
        let victims: Vec<_> = (0..self.players.len())
            .filter(|&index| {
                index != source
                    && self.players[index].raw_fd() >= 0
                    && self.same_tile(self.players[index].position(), origin)
            })
            .collect();
        let ejected = !victims.is_empty();
        for victim in victims {
            let mut destination = origin;
            match direction {
                Direction::North => destination.y += 1,
                Direction::South => destination.y -= 1,
                Direction::Est => destination.x += 1,
                Direction::West => destination.x -= 1,
                Direction::None => {}
            }
            self.players[victim].set_position(destination);
            let tile = self.sound_tile(victim, origin);
            self.push_reply(self.players[victim].raw_fd(), &format!("eject: {tile}\n"));
        }

        let mut destroyed = Vec::new();
        let (width, height) = self.map.dimensions();
        self.eggs.retain(|egg| {
            if (egg.position.x as isize).rem_euclid(width as isize)
                == (origin.x as isize).rem_euclid(width as isize)
                && (egg.position.y as isize).rem_euclid(height as isize)
                    == (origin.y as isize).rem_euclid(height as isize)
            {
                destroyed.push(egg.team.clone());
                false
            } else {
                true
            }
        });
        for team in destroyed {
            *self.team_capacity.get_mut(&team).unwrap() -= 1;
        }
        if ejected {
            "ok\n".into()
        } else {
            "ko\n".into()
        }
    }

    fn vision_vectors(direction: Direction) -> ((isize, isize), (isize, isize)) {
        match direction {
            Direction::North => ((0, 1), (1, 0)),
            Direction::South => ((0, -1), (-1, 0)),
            Direction::Est => ((1, 0), (0, -1)),
            Direction::West => ((-1, 0), (0, 1)),
            Direction::None => ((0, 0), (0, 0)),
        }
    }

    fn tile_content(&self, x: isize, y: isize) -> String {
        let (width, height) = self.map.dimensions();
        let wrapped_x = x.rem_euclid(width as isize);
        let wrapped_y = y.rem_euclid(height as isize);
        let mut objects = Vec::new();

        for player in &self.players {
            let position = player.position();
            if (position.x as isize).rem_euclid(width as isize) == wrapped_x
                && (position.y as isize).rem_euclid(height as isize) == wrapped_y
            {
                objects.push("player");
            }
        }
        for resource in self.map.get(wrapped_x, wrapped_y).resources() {
            objects.push(resource.name());
        }

        objects.join(" ")
    }

    fn look(&self, player_index: usize) -> String {
        let player = &self.players[player_index];
        let position = player.position();
        let (forward, right) = Self::vision_vectors(player.direction());
        let mut tiles = Vec::new();

        for depth in 0..=player.level() as isize {
            for lateral in -depth..=depth {
                let x = position.x as isize + forward.0 * depth + right.0 * lateral;
                let y = position.y as isize + forward.1 * depth + right.1 * lateral;
                tiles.push(self.tile_content(x, y));
            }
        }

        format!("[{}]\n", tiles.join(","))
    }

    pub fn run_ticks(&mut self) {
        self.ticks += 1;
        if self.ticks % REFILL_INTERVAL == 0 {
            self.map.refill();
        }
        self.do_action();
    }

    pub fn deplete(&mut self, resource: Resource, amount: usize) -> usize {
        self.map.deplete(resource, amount)
    }

    pub fn count(&self, resource: Resource) -> usize {
        self.map.count(resource)
    }

    pub fn add_players(&mut self) -> EntityId {
        let id = self.players.len();
        self.players.push(Entity::new_dummy());
        self.players[id].set_id(id);
        id as EntityId
    }

    fn connected_on_team(&self, team: &str) -> usize {
        self.sessions
            .values()
            .filter(|session| {
                matches!(
                    &session.state,
                    SessionState::Ready { team: name, .. } if name == team
                )
            })
            .count()
    }

    fn available_slots(&self, team: &str) -> usize {
        self.team_capacity[team].saturating_sub(self.connected_on_team(team))
    }

    fn accept_team_name(&mut self, client_fd: u64, team_name: &str) -> ClientReply {
        if !self.teams.iter().any(|team| team == team_name) {
            self.sessions.remove(&client_fd);
            return ClientReply::data_then_close(b"ko\n".to_vec());
        }

        let connected = self.connected_on_team(team_name);
        let available = self.available_slots(team_name);
        if available < 1 {
            self.sessions.remove(&client_fd);
            return ClientReply::data_then_close(b"ko\n".to_vec());
        }

        let player_id = self.add_players();
        self.players[player_id as usize].set_raw_fd(client_fd as i32);
        self.players[player_id as usize].set_team(&team_name.to_string());
        if connected >= self.clients_per_team
            && let Some(index) = self.eggs.iter().position(|egg| egg.team == team_name)
        {
            let egg = self.eggs.remove(index);
            self.players[player_id as usize].set_position(egg.position);
        }
        let (width, height) = self.map_dimensions();
        self.sessions.insert(
            client_fd,
            Session {
                state: SessionState::Ready {
                    player_id,
                    team: team_name.to_string(),
                },
            },
        );

        ClientReply::data(format!("{available}\n{width} {height}\n").into_bytes())
    }
}

impl ClientHandler for Game {
    fn tick(&mut self) -> HashMap<i32, String> {
        self.run_ticks();
        std::mem::take(&mut self.reply)
    }

    fn on_connect(&mut self, client_fd: u64) -> Vec<u8> {
        self.sessions.insert(
            client_fd,
            Session {
                state: SessionState::AwaitingTeamName,
            },
        );
        b"WELCOME\n".to_vec()
    }

    fn client_message(&mut self, client_fd: u64, data: &str) -> Option<ClientReply> {
        let Some(session) = self.sessions.get(&client_fd) else {
            return Some(ClientReply::data(b"ko\n".to_vec()));
        };

        match &session.state {
            SessionState::AwaitingTeamName => Some(self.accept_team_name(client_fd, data)),
            SessionState::Ready { team, .. } if data == "Connect_nbr" => Some(ClientReply::data(
                format!("{}\n", self.available_slots(team)).into_bytes(),
            )),
            SessionState::Ready { player_id, .. } => match parse(data) {
                Ok(act) => {
                    if self.players[*player_id as usize].actions.len() >= MAX_ACTION {
                        return Some(ClientReply::data(b"ko\n".to_vec()));
                    }
                    self.players[*player_id as usize].add_action(act);
                    return None;
                }
                Err(_) => Some(ClientReply::data(b"ko\n".to_vec())),
            },
        }
    }

    fn client_disconnect(&mut self, client_fd: u64) {
        if let Some(Session {
            state: SessionState::Ready { player_id, .. },
        }) = self.sessions.remove(&client_fd)
        {
            self.players[player_id as usize].set_raw_fd(-1);
        }
    }
}

#[cfg(test)]
mod action_tests {
    use super::*;
    use crate::action::Action;
    use crate::data::RESOURCES;

    fn game_without_resources(width: usize, height: usize) -> Game {
        let mut game = Game::new(width, height, vec!["team".to_string()], 5);
        for resource in RESOURCES {
            game.deplete(resource, usize::MAX);
        }
        game
    }

    #[test]
    fn look_returns_forward_triangle_with_wrapped_positions() {
        let mut game = game_without_resources(3, 3);
        let viewer = game.add_players() as usize;
        let visible_player = game.add_players() as usize;

        game.players[visible_player].forward();

        assert_eq!(game.look(viewer), "[player,,player,]\n");
    }

    #[test]
    fn look_replies_after_seven_ticks() {
        let mut game = game_without_resources(3, 3);
        let player = game.add_players() as usize;
        game.players[player].set_raw_fd(42);
        game.players[player].add_action(Action::new_look());

        for _ in 0..6 {
            assert!(game.tick().is_empty());
        }

        assert_eq!(game.tick().get(&42), Some(&"[player,,,]\n".to_string()));
    }

    #[test]
    fn forward_waits_seven_ticks_before_moving() {
        let mut game = game_without_resources(3, 3);
        let player = game.add_players() as usize;
        game.players[player].set_raw_fd(10);
        game.players[player].add_action(Action::new_forward());

        for _ in 0..6 {
            assert!(game.tick().is_empty());
            assert_eq!(
                game.players[player].position(),
                crate::data::Position { x: 0, y: 0 }
            );
        }

        assert_eq!(game.tick().get(&10), Some(&"ok\n".to_string()));
        assert_eq!(
            game.players[player].position(),
            crate::data::Position { x: 0, y: -1 }
        );
    }

    #[test]
    fn right_waits_seven_ticks_then_rotates_clockwise() {
        let mut game = game_without_resources(3, 3);
        let player = game.add_players() as usize;
        game.players[player].set_raw_fd(11);
        game.players[player].add_action(Action::new_right());

        for _ in 0..6 {
            assert!(game.tick().is_empty());
            assert_eq!(game.players[player].direction(), Direction::South);
        }

        assert_eq!(game.tick().get(&11), Some(&"ok\n".to_string()));
        assert_eq!(game.players[player].direction(), Direction::West);
    }

    #[test]
    fn left_waits_seven_ticks_then_rotates_counterclockwise() {
        let mut game = game_without_resources(3, 3);
        let player = game.add_players() as usize;
        game.players[player].set_raw_fd(12);
        game.players[player].add_action(Action::new_left());

        for _ in 0..6 {
            assert!(game.tick().is_empty());
            assert_eq!(game.players[player].direction(), Direction::South);
        }

        assert_eq!(game.tick().get(&12), Some(&"ok\n".to_string()));
        assert_eq!(game.players[player].direction(), Direction::Est);
    }

    #[test]
    fn inventory_completes_after_one_tick() {
        let mut game = game_without_resources(3, 3);
        let player = game.add_players() as usize;
        game.players[player].set_raw_fd(13);
        game.players[player].add_action(Action::new_inventory());

        assert_eq!(
            game.tick().get(&13),
            Some(
                &"[food 10, linemate 0, deraumere 0, sibur 0, mendiane 0, phiras 0, thystame 0]\n"
                    .to_string()
            )
        );
        assert!(game.players[player].actions.is_empty());
    }

    #[test]
    fn queued_actions_execute_in_fifo_order() {
        let mut game = game_without_resources(3, 3);
        let player = game.add_players() as usize;
        game.players[player].set_raw_fd(14);
        game.players[player].add_action(Action::new_forward());
        game.players[player].add_action(Action::new_left());

        for _ in 0..7 {
            game.tick();
        }
        assert_eq!(
            game.players[player].position(),
            crate::data::Position { x: 0, y: -1 }
        );
        assert_eq!(game.players[player].direction(), Direction::South);

        for _ in 0..6 {
            assert!(game.tick().is_empty());
            assert_eq!(game.players[player].direction(), Direction::South);
        }
        assert_eq!(game.tick().get(&14), Some(&"ok\n".to_string()));
        assert_eq!(game.players[player].direction(), Direction::Est);
    }

    #[test]
    fn action_queue_accepts_ten_actions_and_rejects_the_eleventh() {
        let mut entity = Entity::new_dummy();

        for _ in 0..10 {
            assert!(entity.add_action(Action::new_forward()));
        }
        assert!(!entity.add_action(Action::new_forward()));
        assert_eq!(entity.actions.len(), 10);
    }

    #[test]
    fn tick_processes_one_action_for_each_player() {
        let mut game = game_without_resources(3, 3);
        let first = game.add_players() as usize;
        let second = game.add_players() as usize;
        game.players[first].set_raw_fd(20);
        game.players[second].set_raw_fd(21);
        game.players[first].add_action(Action::new_inventory());
        game.players[second].add_action(Action::new_inventory());

        let replies = game.tick();

        assert!(replies[&20].starts_with("[food 10,"));
        assert!(replies[&21].starts_with("[food 10,"));
    }

    #[test]
    fn tick_drains_completed_action_replies() {
        let mut game = game_without_resources(3, 3);
        let player = game.add_players() as usize;
        game.players[player].set_raw_fd(22);
        game.players[player].add_action(Action::new_inventory());

        assert!(game.tick()[&22].starts_with("[food 10,"));
        assert!(game.tick().is_empty());
    }

    #[test]
    fn resource_commands_move_one_item_between_tile_and_inventory() {
        let mut game = game_without_resources(3, 3);
        let player = game.add_players() as usize;
        game.players[player].set_raw_fd(40);
        let position = game.players[player].position();
        game.map.put(position, Resource::Linemate);

        game.players[player].add_action(Action::new_take(Resource::Linemate));
        for _ in 0..7 {
            game.tick();
        }
        assert_eq!(
            game.players[player].inventory()[Resource::Linemate as usize],
            1
        );
        assert_eq!(game.count(Resource::Linemate), 0);

        game.players[player].add_action(Action::new_set(Resource::Linemate));
        for _ in 0..7 {
            game.tick();
        }
        assert_eq!(
            game.players[player].inventory()[Resource::Linemate as usize],
            0
        );
        assert_eq!(game.count(Resource::Linemate), 1);
    }

    #[test]
    fn broadcast_and_eject_notify_other_players() {
        let mut game = game_without_resources(3, 3);
        let source = game.add_players() as usize;
        let target = game.add_players() as usize;
        game.players[source].set_raw_fd(41);
        game.players[target].set_raw_fd(42);

        game.players[source].add_action(Action::new_broadcast("hello".into()));
        for _ in 0..6 {
            game.tick();
        }
        let replies = game.tick();
        assert_eq!(replies[&41], "ok\n");
        assert_eq!(replies[&42], "message 0, hello\n");

        game.players[source].add_action(Action::new_eject());
        for _ in 0..6 {
            game.tick();
        }
        let replies = game.tick();
        assert_eq!(replies[&41], "ok\n");
        assert!(replies[&42].starts_with("eject: "));
        assert_ne!(
            game.players[target].position(),
            game.players[source].position()
        );
    }

    #[test]
    fn connect_nbr_is_immediate_and_fork_adds_a_slot_after_42_ticks() {
        let mut game = game_without_resources(3, 3);
        game.on_connect(50);
        game.client_message(50, "team");
        assert_eq!(game.client_message(50, "Connect_nbr").unwrap().data, b"4\n");

        assert!(game.client_message(50, "Fork").is_none());
        for _ in 0..42 {
            game.tick();
        }
        assert_eq!(game.client_message(50, "Connect_nbr").unwrap().data, b"5\n");
    }

    #[test]
    fn parser_accepts_new_commands_and_rejects_incantation() {
        assert!(
            matches!(parse("Broadcast hello"), Ok(action) if action.kind() == EAction::Broadcast("hello".into()))
        );
        assert!(
            matches!(parse("Take food"), Ok(action) if action.kind() == EAction::Take(Resource::Food))
        );
        assert!(
            matches!(parse("Set thystame"), Ok(action) if action.kind() == EAction::Set(Resource::Thystame))
        );
        assert!(parse("Incantation").is_err());
    }

    #[test]
    fn accepted_command_is_queued_without_immediate_reply() {
        let mut game = game_without_resources(3, 3);
        game.on_connect(30);
        assert!(game.client_message(30, "team").is_some());

        assert!(game.client_message(30, "Forward").is_none());
        assert_eq!(game.players[0].actions.len(), 1);
        assert_eq!(game.players[0].actions[0].kind(), EAction::Forward);
    }

    #[test]
    fn full_action_queue_returns_ko_without_adding_command() {
        let mut game = game_without_resources(3, 3);
        game.on_connect(31);
        assert!(game.client_message(31, "team").is_some());
        for _ in 0..10 {
            assert!(game.client_message(31, "Forward").is_none());
        }

        let reply = game
            .client_message(31, "Forward")
            .expect("queue-full reply");

        assert_eq!(reply.data, b"ko\n");
        assert_eq!(game.players[0].actions.len(), 10);
    }
}
