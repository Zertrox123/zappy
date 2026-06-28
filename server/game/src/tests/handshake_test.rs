use server::server::ClientHandler;

use crate::data::Resource;
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

    let reply = game.client_message(3, "team").expect("handshake reply");
    assert!(!reply.disconnect);
    assert_eq!(reply.data, b"5\n10 12\n");
}

#[test]
fn handshake_rejects_unknown_team() {
    let mut game = Game::new(3, 3, vec!["team".to_string()], 5);
    let _ = game.on_connect(4);

    let reply = game.client_message(4, "unknown").expect("rejection reply");
    assert!(reply.disconnect);
    assert_eq!(reply.data, b"ko\n");
}

#[test]
fn handshake_rejects_when_team_is_full() {
    let mut game = Game::new(3, 3, vec!["team".to_string()], 1);
    let _ = game.on_connect(5);
    let first = game.client_message(5, "team").expect("handshake reply");
    assert!(!first.disconnect);

    let _ = game.on_connect(6);
    let second = game.client_message(6, "team").expect("team-full reply");
    assert!(second.disconnect);
    assert_eq!(second.data, b"ko\n");
}

#[test]
fn handshake_decrements_available_slots_per_connection() {
    let mut game = Game::new(4, 6, vec!["team".to_string()], 3);
    let _ = game.on_connect(7);
    let first = game.client_message(7, "team").expect("handshake reply");
    assert_eq!(first.data, b"3\n4 6\n");

    let _ = game.on_connect(8);
    let second = game.client_message(8, "team").expect("handshake reply");
    assert_eq!(second.data, b"2\n4 6\n");
}

#[test]
fn handshake_keys_sessions_by_client_fd() {
    let mut game = Game::new(3, 3, vec!["team".to_string()], 2);
    let _ = game.on_connect(100);
    let handshake = game.client_message(100, "team").expect("handshake reply");
    assert!(!handshake.disconnect);

    let unknown = game
        .client_message(101, "Look")
        .expect("unknown-client reply");
    assert_eq!(unknown.data, b"ko\n");
}

#[test]
fn graphic_handshake_enables_gui_protocol_commands() {
    let mut game = Game::new(2, 3, vec!["red".to_string(), "blue".to_string()], 5);
    let _ = game.on_connect(42);

    let init = game.client_message(42, "GRAPHIC").expect("graphic reply");
    let init = String::from_utf8(init.data).expect("utf8 gui init");
    assert!(init.starts_with("msz 2 3\n"));
    assert!(init.contains("tna red\n"));
    assert!(init.contains("tna blue\n"));
    assert!(init.contains("smg GUI connected\n"));
    assert_eq!(init.matches("bct ").count(), 6);

    let map_size = game.client_message(42, "msz").expect("msz reply");
    assert_eq!(map_size.data, b"msz 2 3\n");

    let bad_params = game.client_message(42, "bct 9 9").expect("bct reply");
    assert_eq!(bad_params.data, b"sbp\n");
}

#[test]
fn multiple_graphic_clients_receive_player_events() {
    let mut game = Game::new(3, 3, vec!["team".to_string()], 5);
    let _ = game.on_connect(10);
    let _ = game.client_message(10, "GRAPHIC");
    let _ = game.on_connect(11);
    let _ = game.client_message(11, "GRAPHIC");
    let _ = game.on_connect(20);

    let _ = game.client_message(20, "team");
    let replies = game.tick();

    assert!(replies[&10].contains("pnw #0 0 0 3 1 team\n"));
    assert!(replies[&11].contains("pnw #0 0 0 3 1 team\n"));
}

#[test]
fn graphic_refill_updates_only_changed_tiles() {
    let mut game = Game::new(3, 3, vec!["team".to_string()], 5);
    let _ = game.on_connect(10);
    let _ = game.client_message(10, "GRAPHIC");
    game.deplete(Resource::Food, 1);

    for _ in 0..19 {
        assert!(game.tick().is_empty());
    }

    let replies = game.tick();
    assert_eq!(replies[&10].matches("bct ").count(), 1);
}
