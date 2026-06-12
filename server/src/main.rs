use std::io::{self, Write};
use std::process;

use zappy_server::config::{self, EXIT_USAGE, USAGE};
use zappy_server::server::Server;

fn print_usage() {
    let _ = io::stdout().write_all(USAGE.as_bytes());
}

fn main() {
    let args: Vec<String> = std::env::args().collect();
    if args.iter().any(|arg| arg == "--help" || arg == "-h") {
        print_usage();
        return;
    }

    let config = match config::parse_args(args) {
        Ok(config) => config,
        Err(err) => {
            eprintln!("{err}");
            process::exit(EXIT_USAGE);
        }
    };

    let mut server = Server::new(&config).unwrap_or_else(|err| {
        eprintln!("{err}");
        process::exit(EXIT_USAGE);
    });

    if let Err(err) = server.run() {
        eprintln!("{err}");
        process::exit(EXIT_USAGE);
    }
}
