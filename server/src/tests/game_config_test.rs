use crate::config::ServerConfig;
use crate::game::Game;

fn config_with_map(width: usize, height: usize) -> ServerConfig {
    ServerConfig {
        port: 8080,
        width,
        height,
        teams: vec!["team".to_string()],
        clients_per_team: 5,
        frequency: 100,
    }
}

#[test]
fn game_builds_map_from_config_dimensions() {
    let game = Game::new(&config_with_map(42, 17));
    assert_eq!(game.map_dimensions(), (42, 17));
}

#[test]
fn game_builds_small_map_from_config() {
    let game = Game::new(&config_with_map(3, 4));
    assert_eq!(game.map_dimensions(), (3, 4));
}

#[test]
fn game_stores_teams_from_config() {
    let config = ServerConfig {
        port: 8080,
        width: 5,
        height: 5,
        teams: vec!["team1".to_string(), "team2".to_string()],
        clients_per_team: 5,
        frequency: 100,
    };
    let game = Game::new(&config);
    assert_eq!(game.teams(), &["team1", "team2"]);
}

#[test]
fn game_stores_clients_per_team_from_config() {
    let mut config = config_with_map(5, 5);
    config.clients_per_team = 12;
    let game = Game::new(&config);
    assert_eq!(game.clients_per_team(), 12);
}

#[test]
fn game_exam_configuration_dimensions() {
    let config = ServerConfig {
        port: 8080,
        width: 42,
        height: 42,
        teams: vec!["team".to_string()],
        clients_per_team: 5,
        frequency: 100,
    };
    let game = Game::new(&config);
    assert_eq!(game.map_dimensions(), (42, 42));
    assert_eq!(game.teams(), &["team"]);
    assert_eq!(game.clients_per_team(), 5);
}

#[test]
fn game_teams_are_cloned_independently_from_config() {
    let mut config = config_with_map(2, 2);
    config.teams.push("extra".to_string());
    let game = Game::new(&config);
    assert_eq!(game.teams().len(), 2);
    assert_eq!(game.teams()[1], "extra");
}
