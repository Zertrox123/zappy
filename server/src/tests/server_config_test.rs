use crate::config::ServerConfig;
use crate::server::Server;

fn config_on_port(port: u16, frequency: u32) -> ServerConfig {
    ServerConfig {
        port,
        width: 3,
        height: 3,
        teams: vec!["team".to_string()],
        clients_per_team: 1,
        frequency,
    }
}

#[test]
fn server_uses_frequency_as_tickrate() {
    let port = reserve_port();
    let server = Server::new(&config_on_port(port, 250)).expect("server should bind");
    assert_eq!(server.tickrate(), 250);
}

#[test]
fn server_binds_to_configured_port() {
    let port = reserve_port();
    let server = Server::new(&config_on_port(port, 100)).expect("server should bind");
    assert_eq!(server.bound_port(), port);
}

#[test]
fn server_exam_configuration_tickrate() {
    let port = reserve_port();
    let config = ServerConfig {
        port,
        width: 42,
        height: 42,
        teams: vec!["team".to_string()],
        clients_per_team: 5,
        frequency: 100,
    };
    let server = Server::new(&config).expect("server should bind");
    assert_eq!(server.tickrate(), 100);
    assert_eq!(server.bound_port(), port);
}

#[test]
fn server_fails_when_port_is_already_in_use() {
    let port = reserve_port();
    let holder = std::net::TcpListener::bind(format!("0.0.0.0:{port}"))
        .expect("reserve port for conflict test");
    let result = Server::new(&config_on_port(port, 100));
    drop(holder);
    assert!(result.is_err());
}

fn reserve_port() -> u16 {
    std::net::TcpListener::bind("127.0.0.1:0")
        .expect("ephemeral port")
        .local_addr()
        .expect("local addr")
        .port()
}
