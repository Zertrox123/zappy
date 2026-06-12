use std::io::Write;
use std::process;

use server::config::{self, EXIT_USAGE, USAGE};
use server::server::Server;

use game::game::Game;

fn main() {
    let args: Vec<String> = std::env::args().collect();
    if args.iter().any(|arg| arg == "--help" || arg == "-h") {
        let _ = std::io::stdout().write_all(USAGE.as_bytes());
        return;
    }

    let config = match config::parse_args(args) {
        Ok(config) => config,
        Err(err) => {
            eprintln!("{err}");
            process::exit(EXIT_USAGE);
        }
    };

    let game = Game::new(
        config.width,
        config.height,
        config.teams.clone(),
        config.clients_per_team,
    );

    let mut server = match Server::new(&config, game) {
        Ok(server) => server,
        Err(err) => {
            eprintln!("{err}");
            process::exit(EXIT_USAGE);
        }
    };

    if let Err(err) = server.run() {
        eprintln!("{err}");
        process::exit(EXIT_USAGE);
    }
}
