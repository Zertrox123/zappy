use super::*;
use crate::action::Action;
use crate::data::{RESOURCES, parse};
use server::server::ClientHandler;

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
            &"[food 9, linemate 0, deraumere 0, sibur 0, mendiane 0, phiras 0, thystame 0]\n"
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

    assert!(replies[&20].starts_with("[food 9,"));
    assert!(replies[&21].starts_with("[food 9,"));
}

#[test]
fn tick_drains_completed_action_replies() {
    let mut game = game_without_resources(3, 3);
    let player = game.add_players() as usize;
    game.players[player].set_raw_fd(22);
    game.players[player].add_action(Action::new_inventory());

    assert!(game.tick()[&22].starts_with("[food 9,"));
    assert!(game.tick().is_empty());
}

#[test]
fn player_dies_when_life_time_and_food_are_empty() {
    let mut game = game_without_resources(3, 3);
    let player = game.add_players() as usize;
    game.players[player].set_raw_fd(23);
    game.players[player].set_saturation(0);
    game.players[player].inventory_mut()[Resource::Food as usize] = 0;

    assert_eq!(game.tick().get(&23), Some(&"dead\n".to_string()));
    assert!(game.players.is_empty());
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
fn parser_accepts_new_commands() {
    assert!(
        matches!(parse("Broadcast hello"), Ok(action) if action.kind() == EAction::Broadcast("hello".into()))
    );
    assert!(
        matches!(parse("Take food"), Ok(action) if action.kind() == EAction::Take(Resource::Food))
    );
    assert!(
        matches!(parse("Set thystame"), Ok(action) if action.kind() == EAction::Set(Resource::Thystame))
    );
    assert!(matches!(
        parse("Incantation"),
        Ok(action) if matches!(action.kind(), EAction::Incantation { .. })
    ));
}

#[test]
fn incantation_levels_player_after_three_hundred_ticks() {
    let mut game = game_without_resources(3, 3);
    game.on_connect(60);
    game.client_message(60, "team");
    let position = game.players[0].position();
    game.map.put(position, Resource::Linemate);

    let reply = game.client_message(60, "Incantation").unwrap();
    assert_eq!(reply.data, b"Elevation underway\n");

    for _ in 0..299 {
        assert!(game.tick().is_empty());
    }

    assert_eq!(
        game.tick().get(&60),
        Some(&"Current level: 2\n".to_string())
    );
    assert_eq!(game.players[0].level(), 2);
}

#[test]
fn incantation_sends_seg_when_team_reaches_win_condition() {
    let mut game = game_without_resources(3, 3);
    let _ = game.on_connect(70);
    let _ = game.client_message(70, "GRAPHIC");
    let position = Position { x: 0, y: 0 };

    for fd in 80..86 {
        let player = game.add_players() as usize;
        game.players[player].set_raw_fd(fd);
        game.players[player].set_team(&"team".to_string());
        game.players[player].set_position(position);
        for _ in 0..6 {
            game.players[player].level_up();
        }
    }

    for (resource, amount) in [
        (Resource::Linemate, 2),
        (Resource::Deraumere, 2),
        (Resource::Sibur, 2),
        (Resource::Mendiane, 2),
        (Resource::Phiras, 2),
        (Resource::Thystame, 1),
    ] {
        for _ in 0..amount {
            game.map.put(position, resource);
        }
    }

    let participants = (0..6).collect::<Vec<_>>();
    assert_eq!(
        game.finish_incantation(position, 7, &participants),
        "Current level: 8\n"
    );

    assert!(game.tick()[&70].contains("seg team\n"));
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
