use crate::server::Server;

pub mod action;
pub mod data;
pub mod server;
pub mod tests;
pub mod game;

fn main() {
    let mut serv = Server::new("0.0.0.0:8080", 1).unwrap();
    serv.run();
    let mut map = data::Map::new(50, 50);
    map.show_map();
}
