use crate::config::{parse_args, ConfigError, ServerConfig, EXIT_USAGE, USAGE};

fn argv(args: &[&str]) -> Vec<String> {
    std::iter::once("zappy_server".to_string())
        .chain(args.iter().map(|s| s.to_string()))
        .collect()
}

fn sample_config() -> ServerConfig {
    ServerConfig {
        port: 8080,
        width: 10,
        height: 10,
        teams: vec!["team".to_string()],
        clients_per_team: 5,
        frequency: 100,
    }
}

#[test]
fn parse_exam_configuration() {
    let config = parse_args(argv(&[
        "-p", "8080", "-x", "42", "-y", "42", "-n", "team", "-c", "5", "-f", "100",
    ]))
    .unwrap();

    assert_eq!(
        config,
        ServerConfig {
            port: 8080,
            width: 42,
            height: 42,
            teams: vec!["team".to_string()],
            clients_per_team: 5,
            frequency: 100,
        }
    );
}

#[test]
fn parse_multiple_teams_after_single_n_flag() {
    let config = parse_args(argv(&[
        "-p", "4242", "-x", "10", "-y", "10", "-n", "team1", "team2", "team3", "-c", "6", "-f",
        "50",
    ]))
    .unwrap();

    assert_eq!(config.teams, vec!["team1", "team2", "team3"]);
    assert_eq!(config.clients_per_team, 6);
}

#[test]
fn parse_teams_from_repeated_n_flags() {
    let config = parse_args(argv(&[
        "-p", "8080", "-x", "10", "-y", "10", "-n", "alpha", "-n", "beta", "-c", "2", "-f", "10",
    ]))
    .unwrap();

    assert_eq!(config.teams, vec!["alpha", "beta"]);
}

#[test]
fn parse_flags_in_different_order() {
    let config = parse_args(argv(&[
        "-f", "75", "-y", "8", "-n", "team", "-x", "12", "-c", "3", "-p", "9000",
    ]))
    .unwrap();

    assert_eq!(config.port, 9000);
    assert_eq!(config.width, 12);
    assert_eq!(config.height, 8);
    assert_eq!(config.frequency, 75);
    assert_eq!(config.clients_per_team, 3);
}

#[test]
fn parse_max_valid_port() {
    let config = parse_args(argv(&[
        "-p", "65535", "-x", "1", "-y", "1", "-n", "team", "-c", "1", "-f", "1",
    ]))
    .unwrap();

    assert_eq!(config.port, 65535);
}

#[test]
fn parse_team_names_with_underscores_and_digits() {
    let config = parse_args(argv(&[
        "-p", "8080", "-x", "5", "-y", "5", "-n", "team_1", "Team2", "-c", "1", "-f", "1",
    ]))
    .unwrap();

    assert_eq!(config.teams, vec!["team_1", "Team2"]);
}

#[test]
fn parse_no_arguments() {
    let err = parse_args(argv(&[])).unwrap_err();
    assert_eq!(err, ConfigError::MissingFlag("-p"));
}

#[test]
fn parse_missing_port() {
    let err = parse_args(argv(&[
        "-x", "10", "-y", "10", "-n", "team", "-c", "5", "-f", "100",
    ]))
    .unwrap_err();
    assert_eq!(err, ConfigError::MissingFlag("-p"));
}

#[test]
fn parse_missing_width() {
    let err = parse_args(argv(&[
        "-p", "8080", "-y", "10", "-n", "team", "-c", "5", "-f", "100",
    ]))
    .unwrap_err();
    assert_eq!(err, ConfigError::MissingFlag("-x"));
}

#[test]
fn parse_missing_height() {
    let err = parse_args(argv(&[
        "-p", "8080", "-x", "10", "-n", "team", "-c", "5", "-f", "100",
    ]))
    .unwrap_err();
    assert_eq!(err, ConfigError::MissingFlag("-y"));
}

#[test]
fn parse_missing_clients() {
    let err = parse_args(argv(&[
        "-p", "8080", "-x", "10", "-y", "10", "-n", "team", "-f", "100",
    ]))
    .unwrap_err();
    assert_eq!(err, ConfigError::MissingFlag("-c"));
}

#[test]
fn parse_missing_frequency() {
    let err = parse_args(argv(&[
        "-p", "8080", "-x", "10", "-y", "10", "-n", "team", "-c", "5",
    ]))
    .unwrap_err();
    assert_eq!(err, ConfigError::MissingFlag("-f"));
}

#[test]
fn parse_missing_team_names() {
    let err = parse_args(argv(&[
        "-p", "8080", "-x", "10", "-y", "10", "-c", "5", "-f", "100",
    ]))
    .unwrap_err();
    assert_eq!(err, ConfigError::MissingFlag("-n"));
}

#[test]
fn parse_n_flag_without_team_name() {
    let err = parse_args(argv(&[
        "-p", "8080", "-x", "10", "-y", "10", "-n", "-c", "5", "-f", "100",
    ]))
    .unwrap_err();
    assert_eq!(err, ConfigError::NoTeams);
}

#[test]
fn parse_empty_team_name() {
    let err = parse_args(argv(&[
        "-p", "8080", "-x", "10", "-y", "10", "-n", "team", "", "other", "-c", "5", "-f", "100",
    ]))
    .unwrap_err();
    assert_eq!(err, ConfigError::EmptyTeamName);
}

#[test]
fn parse_missing_value_for_port() {
    let err = parse_args(argv(&["-p"])).unwrap_err();
    assert_eq!(err, ConfigError::MissingValue { flag: "-p" });
}

#[test]
fn parse_missing_value_for_width() {
    let err = parse_args(argv(&["-x"])).unwrap_err();
    assert_eq!(err, ConfigError::MissingValue { flag: "-x" });
}

#[test]
fn parse_missing_value_for_height() {
    let err = parse_args(argv(&["-y"])).unwrap_err();
    assert_eq!(err, ConfigError::MissingValue { flag: "-y" });
}

#[test]
fn parse_missing_value_for_clients() {
    let err = parse_args(argv(&["-c"])).unwrap_err();
    assert_eq!(err, ConfigError::MissingValue { flag: "-c" });
}

#[test]
fn parse_missing_value_for_frequency() {
    let err = parse_args(argv(&["-f"])).unwrap_err();
    assert_eq!(err, ConfigError::MissingValue { flag: "-f" });
}

#[test]
fn parse_invalid_port_zero() {
    let err = parse_args(argv(&[
        "-p", "0", "-x", "10", "-y", "10", "-n", "team", "-c", "5", "-f", "100",
    ]))
    .unwrap_err();
    assert_eq!(
        err,
        ConfigError::InvalidValue {
            flag: "-p",
            value: "0".to_string(),
        }
    );
}

#[test]
fn parse_invalid_port_non_numeric() {
    let err = parse_args(argv(&[
        "-p", "abc", "-x", "10", "-y", "10", "-n", "team", "-c", "5", "-f", "100",
    ]))
    .unwrap_err();
    assert_eq!(
        err,
        ConfigError::InvalidValue {
            flag: "-p",
            value: "abc".to_string(),
        }
    );
}

#[test]
fn parse_invalid_port_above_u16_max() {
    let err = parse_args(argv(&[
        "-p", "65536", "-x", "10", "-y", "10", "-n", "team", "-c", "5", "-f", "100",
    ]))
    .unwrap_err();
    assert_eq!(
        err,
        ConfigError::InvalidValue {
            flag: "-p",
            value: "65536".to_string(),
        }
    );
}

#[test]
fn parse_invalid_map_width() {
    let err = parse_args(argv(&[
        "-p", "8080", "-x", "0", "-y", "10", "-n", "team", "-c", "5", "-f", "100",
    ]))
    .unwrap_err();
    assert_eq!(
        err,
        ConfigError::InvalidValue {
            flag: "-x",
            value: "0".to_string(),
        }
    );
}

#[test]
fn parse_invalid_map_height() {
    let err = parse_args(argv(&[
        "-p", "8080", "-x", "10", "-y", "-3", "-n", "team", "-c", "5", "-f", "100",
    ]))
    .unwrap_err();
    assert_eq!(
        err,
        ConfigError::InvalidValue {
            flag: "-y",
            value: "-3".to_string(),
        }
    );
}

#[test]
fn parse_invalid_clients_zero() {
    let err = parse_args(argv(&[
        "-p", "8080", "-x", "10", "-y", "10", "-n", "team", "-c", "0", "-f", "100",
    ]))
    .unwrap_err();
    assert_eq!(
        err,
        ConfigError::InvalidValue {
            flag: "-c",
            value: "0".to_string(),
        }
    );
}

#[test]
fn parse_invalid_clients_non_numeric() {
    let err = parse_args(argv(&[
        "-p", "8080", "-x", "10", "-y", "10", "-n", "team", "-c", "many", "-f", "100",
    ]))
    .unwrap_err();
    assert_eq!(
        err,
        ConfigError::InvalidValue {
            flag: "-c",
            value: "many".to_string(),
        }
    );
}

#[test]
fn parse_invalid_frequency_zero() {
    let err = parse_args(argv(&[
        "-p", "8080", "-x", "10", "-y", "10", "-n", "team", "-c", "5", "-f", "0",
    ]))
    .unwrap_err();
    assert_eq!(
        err,
        ConfigError::InvalidValue {
            flag: "-f",
            value: "0".to_string(),
        }
    );
}

#[test]
fn parse_invalid_frequency_non_numeric() {
    let err = parse_args(argv(&[
        "-p", "8080", "-x", "10", "-y", "10", "-n", "team", "-c", "5", "-f", "abc",
    ]))
    .unwrap_err();
    assert_eq!(
        err,
        ConfigError::InvalidValue {
            flag: "-f",
            value: "abc".to_string(),
        }
    );
}

#[test]
fn parse_unknown_flag() {
    let err = parse_args(argv(&[
        "-p", "8080", "-x", "10", "-y", "10", "-n", "team", "-c", "5", "-f", "100", "-z", "1",
    ]))
    .unwrap_err();
    assert_eq!(err, ConfigError::UnknownFlag("-z".to_string()));
}

#[test]
fn parse_help_flag_is_rejected_by_parser() {
    let err = parse_args(argv(&["--help"])).unwrap_err();
    assert_eq!(err, ConfigError::UnknownFlag("--help".to_string()));
}

#[test]
fn config_error_display_messages() {
    assert_eq!(
        ConfigError::MissingFlag("-p").to_string(),
        "missing required argument: -p"
    );
    assert_eq!(
        ConfigError::MissingValue { flag: "-f" }.to_string(),
        "missing value for -f"
    );
    assert_eq!(
        ConfigError::InvalidValue {
            flag: "-p",
            value: "bad".to_string(),
        }
        .to_string(),
        "invalid value for -p: bad"
    );
}

#[test]
fn usage_string_matches_subject_format() {
    assert!(USAGE.contains("USAGE: ./zappy_server -p port -x width -y height -n name1 name2"));
    assert!(USAGE.contains("-c clientsNb"));
    assert!(USAGE.contains("-f freq"));
}

#[test]
fn exit_usage_code_is_84() {
    assert_eq!(EXIT_USAGE, 84);
}

#[test]
fn sample_config_matches_manual_construction() {
    let parsed = parse_args(argv(&[
        "-p", "8080", "-x", "10", "-y", "10", "-n", "team", "-c", "5", "-f", "100",
    ]))
    .unwrap();
    assert_eq!(parsed, sample_config());
}
