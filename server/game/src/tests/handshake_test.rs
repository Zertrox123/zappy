use server::server::ClientHandler;

use crate::game::Game;

#[test]
fn on_connect_sends_welcome() {
    let mut game = Game::new(10, 12, vec!["team".to_string()], 5);
    let welcome = game.on_connect(42);
    assert_eq!(welcome, b"WELCOME\n");
}

#[test]
fn handshake_accepts_known_team_and_returns_slots_and_map_size() {
    let mut game = Game::new(10, 12, vec!["team".to_string()], 5);
    let _ = game.on_connect(3);

    let reply = game.client_message(3, "team");
    assert!(!reply.disconnect);
    assert_eq!(reply.data, b"5\n10 12\n");
}

#[test]
fn handshake_rejects_unknown_team() {
    let mut game = Game::new(3, 3, vec!["team".to_string()], 5);
    let _ = game.on_connect(4);

    let reply = game.client_message(4, "unknown");
    assert!(reply.disconnect);
    assert_eq!(reply.data, b"ko\n");
}

#[test]
fn handshake_rejects_when_team_is_full() {
    let mut game = Game::new(3, 3, vec!["team".to_string()], 1);
    let _ = game.on_connect(5);
    let first = game.client_message(5, "team");
    assert!(!first.disconnect);

    let _ = game.on_connect(6);
    let second = game.client_message(6, "team");
    assert!(second.disconnect);
    assert_eq!(second.data, b"ko\n");
}

#[test]
fn handshake_decrements_available_slots_per_connection() {
    let mut game = Game::new(4, 6, vec!["team".to_string()], 3);
    let _ = game.on_connect(7);
    let first = game.client_message(7, "team");
    assert_eq!(first.data, b"3\n4 6\n");

    let _ = game.on_connect(8);
    let second = game.client_message(8, "team");
    assert_eq!(second.data, b"2\n4 6\n");
}

#[test]
fn handshake_keys_sessions_by_client_fd() {
    let mut game = Game::new(3, 3, vec!["team".to_string()], 2);
    let _ = game.on_connect(100);
    let handshake = game.client_message(100, "team");
    assert!(!handshake.disconnect);

    let unknown = game.client_message(101, "Look");
    assert_eq!(unknown.data, b"ko\n");
}
