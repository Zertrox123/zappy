use std::net::{TcpListener, TcpStream};
use std::process::Command;
use std::time::Duration;

use zappy_server::config::{EXIT_USAGE, USAGE};

fn server_bin() -> &'static str {
    env!("CARGO_BIN_EXE_zappy_server")
}

fn reserve_port() -> u16 {
    TcpListener::bind("127.0.0.1:0")
        .expect("ephemeral port")
        .local_addr()
        .expect("local addr")
        .port()
}

#[test]
fn help_flag_prints_usage_and_exits_zero() {
    let output = Command::new(server_bin())
        .arg("--help")
        .output()
        .expect("run zappy_server --help");

    assert!(output.status.success());
    assert_eq!(String::from_utf8_lossy(&output.stdout), USAGE);
    assert!(output.stderr.is_empty());
}

#[test]
fn short_help_flag_prints_usage_and_exits_zero() {
    let output = Command::new(server_bin())
        .arg("-h")
        .output()
        .expect("run zappy_server -h");

    assert!(output.status.success());
    assert_eq!(String::from_utf8_lossy(&output.stdout), USAGE);
}

#[test]
fn no_arguments_exits_with_usage_code() {
    let output = Command::new(server_bin())
        .output()
        .expect("run zappy_server without args");

    assert_eq!(output.status.code(), Some(EXIT_USAGE));
    let stderr = String::from_utf8_lossy(&output.stderr);
    assert!(stderr.contains("missing required argument: -p"));
}

#[test]
fn invalid_port_exits_with_usage_code() {
    let output = Command::new(server_bin())
        .args([
            "-p", "0", "-x", "10", "-y", "10", "-n", "team", "-c", "5", "-f", "100",
        ])
        .output()
        .expect("run zappy_server with invalid port");

    assert_eq!(output.status.code(), Some(EXIT_USAGE));
    assert!(String::from_utf8_lossy(&output.stderr).contains("invalid value for -p: 0"));
}

#[test]
fn missing_team_exits_with_usage_code() {
    let output = Command::new(server_bin())
        .args(["-p", "8080", "-x", "10", "-y", "10", "-c", "5", "-f", "100"])
        .output()
        .expect("run zappy_server without -n");

    assert_eq!(output.status.code(), Some(EXIT_USAGE));
    assert!(String::from_utf8_lossy(&output.stderr).contains("missing required argument: -n"));
}

#[test]
fn exam_configuration_accepts_connections() {
    let port = reserve_port();
    let mut child = Command::new(server_bin())
        .args([
            "-p",
            &port.to_string(),
            "-x",
            "3",
            "-y",
            "3",
            "-n",
            "team",
            "-c",
            "5",
            "-f",
            "100",
        ])
        .spawn()
        .expect("spawn zappy_server with exam configuration");

    std::thread::sleep(Duration::from_millis(300));

    let connect_result = TcpStream::connect(format!("127.0.0.1:{port}"));
    let _ = child.kill();
    let _ = child.wait();

    assert!(
        connect_result.is_ok(),
        "server should listen on configured port"
    );
}

#[test]
fn exam_configuration_does_not_exit_immediately() {
    let port = reserve_port();
    let mut child = Command::new(server_bin())
        .args([
            "-p",
            &port.to_string(),
            "-x",
            "3",
            "-y",
            "3",
            "-n",
            "team",
            "-c",
            "1",
            "-f",
            "100",
        ])
        .spawn()
        .expect("spawn zappy_server");

    std::thread::sleep(Duration::from_millis(200));
    let status = child.try_wait().expect("poll child process");

    let _ = child.kill();
    let _ = child.wait();

    assert!(
        status.is_none(),
        "valid configuration should keep the server running"
    );
}

#[test]
fn bind_failure_exits_with_usage_code() {
    let port = reserve_port();
    let holder = TcpListener::bind(format!("0.0.0.0:{port}"))
        .expect("reserve port for bind failure test");

    let output = Command::new(server_bin())
        .args([
            "-p",
            &port.to_string(),
            "-x",
            "3",
            "-y",
            "3",
            "-n",
            "team",
            "-c",
            "1",
            "-f",
            "100",
        ])
        .output()
        .expect("run zappy_server on used port");

    drop(holder);

    assert_eq!(output.status.code(), Some(EXIT_USAGE));
    assert!(!output.stderr.is_empty());
}

#[test]
fn multiple_teams_argument_is_accepted() {
    let port = reserve_port();
    let mut child = Command::new(server_bin())
        .args([
            "-p",
            &port.to_string(),
            "-x",
            "3",
            "-y",
            "3",
            "-n",
            "team1",
            "team2",
            "-c",
            "1",
            "-f",
            "100",
        ])
        .spawn()
        .expect("spawn zappy_server with multiple teams");

    std::thread::sleep(Duration::from_millis(200));
    let status = child.try_wait().expect("poll child process");

    let _ = child.kill();
    let _ = child.wait();

    assert!(
        status.is_none(),
        "multiple teams after -n should start the server instead of exiting with usage error"
    );
}
