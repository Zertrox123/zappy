use crate::server::Server;

pub mod action;
pub mod data;
pub mod game;
pub mod server;
pub mod tests;

fn main() {
    let mut serv = Server::new("0.0.0.0:8080", 128).unwrap();
    let _ = serv.run();
}
