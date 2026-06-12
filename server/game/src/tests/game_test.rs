use crate::game::Game;

#[test]
fn game_builds_map_from_config_dimensions() {
    let game = Game::new(42, 17, vec!["team".to_string()], 5);
    assert_eq!(game.map_dimensions(), (42, 17));
}

#[test]
fn game_builds_small_map_from_config() {
    let game = Game::new(3, 4, vec!["team".to_string()], 5);
    assert_eq!(game.map_dimensions(), (3, 4));
}

#[test]
fn game_stores_teams_from_config() {
    let game = Game::new(5, 5, vec!["team1".to_string(), "team2".to_string()], 5);
    assert_eq!(game.teams(), &["team1", "team2"]);
}

#[test]
fn game_stores_clients_per_team_from_config() {
    let game = Game::new(5, 5, vec!["team".to_string()], 12);
    assert_eq!(game.clients_per_team(), 12);
}

#[test]
fn game_exam_configuration_dimensions() {
    let game = Game::new(42, 42, vec!["team".to_string()], 5);
    assert_eq!(game.map_dimensions(), (42, 42));
    assert_eq!(game.teams(), &["team"]);
    assert_eq!(game.clients_per_team(), 5);
}

#[test]
fn game_teams_are_cloned_independently() {
    let mut teams = vec!["base".to_string()];
    teams.push("extra".to_string());
    let game = Game::new(2, 2, teams, 5);
    assert_eq!(game.teams().len(), 2);
    assert_eq!(game.teams()[1], "extra");
}
