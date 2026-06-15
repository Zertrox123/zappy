use server::server::ClientHandler;

use crate::game::Game;

#[test]
fn on_connect_sends_welcome() {
    let mut game = Game::new(10, 12, vec!["team".to_string()], 5);
    let (id, welcome) = game.on_connect();
    assert_eq!(id, 0);
    assert_eq!(welcome, b"WELCOME\n");
}

#[test]
fn handshake_accepts_known_team_and_returns_slots_and_map_size() {
    let mut game = Game::new(10, 12, vec!["team".to_string()], 5);
    let (id, _) = game.on_connect();

    let reply = game.client_message(id, "team");
    assert!(!reply.disconnect);
    assert_eq!(reply.data, b"5\n10 12\n");
}

#[test]
fn handshake_rejects_unknown_team() {
    let mut game = Game::new(3, 3, vec!["team".to_string()], 5);
    let (id, _) = game.on_connect();

    let reply = game.client_message(id, "unknown");
    assert!(reply.disconnect);
    assert_eq!(reply.data, b"ko\n");
}

#[test]
fn handshake_rejects_when_team_is_full() {
    let mut game = Game::new(3, 3, vec!["team".to_string()], 1);
    let (first_id, _) = game.on_connect();
    let first = game.client_message(first_id, "team");
    assert!(!first.disconnect);

    let (second_id, _) = game.on_connect();
    let second = game.client_message(second_id, "team");
    assert!(second.disconnect);
    assert_eq!(second.data, b"ko\n");
}

#[test]
fn handshake_decrements_available_slots_per_connection() {
    let mut game = Game::new(4, 6, vec!["team".to_string()], 3);
    let (first_id, _) = game.on_connect();
    let first = game.client_message(first_id, "team");
    assert_eq!(first.data, b"3\n4 6\n");

    let (second_id, _) = game.on_connect();
    let second = game.client_message(second_id, "team");
    assert_eq!(second.data, b"2\n4 6\n");
}

#[test]
fn handshake_allows_commands_after_success() {
    let mut game = Game::new(3, 3, vec!["team".to_string()], 2);
    let (id, _) = game.on_connect();
    let handshake = game.client_message(id, "team");
    assert!(!handshake.disconnect);

    let look = game.client_message(id, "Look");
    assert_eq!(look.data, b"ok\n");
}
