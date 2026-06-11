use std::collections::HashMap;
use std::io::{self, Read, Write};
use std::net::TcpListener;
use std::os::unix::io::{AsRawFd, FromRawFd, RawFd};
use std::time::{Duration, SystemTime};

use libc::{EOF, printf};

use crate::config::ServerConfig;
use crate::data::{self, Entity, EntityId};
use crate::game::Game;

#[derive(Debug)]
pub struct Server {
    listener: TcpListener,
    players: HashMap<RawFd, EntityId>,
    time: SystemTime,
    tickrate: usize,
    game: Game,
}

impl Server {
    pub fn new(config: &ServerConfig) -> io::Result<Server> {
        let addr = format!("0.0.0.0:{}", config.port);
        let listener = TcpListener::bind(&addr)?;
        listener.set_nonblocking(true)?;

        Ok(Server {
            listener,
            players: HashMap::new(),
            time: SystemTime::now(),
            tickrate: config.frequency as usize,
            game: Game::new(config),
        })
    }

    pub fn run(&mut self) -> io::Result<()> {
        println!("Server listening...");

        loop {
            let timestamp = SystemTime::now();
            if timestamp.duration_since(self.time).unwrap()
                > Duration::from_millis((1000 / self.tickrate) as u64)
            {
                self.time = timestamp;
                self.game.run_ticks();
            }

            let mut fds: Vec<libc::pollfd> = Vec::new();
            fds.push(libc::pollfd {
                fd: self.listener.as_raw_fd(),
                events: libc::POLLIN,
                revents: 0,
            });

            for &fd in self.players.keys() {
                fds.push(libc::pollfd {
                    fd,
                    events: libc::POLLIN,
                    revents: 0,
                });
            }

            let n_events = unsafe { libc::poll(fds.as_mut_ptr(), fds.len() as libc::nfds_t, 1000 / self.tickrate as i32) };

            if n_events < 0 {
                return Err(io::Error::last_os_error());
            }

            for pfd in fds.iter() {
                if pfd.revents & libc::POLLIN != 0 {
                    if pfd.fd == self.listener.as_raw_fd() {
                        self.accept_connections()?;
                    } else {
                        self.handle_client_data(pfd.fd)?;
                    }
                } else if pfd.revents & (libc::POLLHUP | libc::POLLERR) != 0 {
                    if pfd.fd != self.listener.as_raw_fd() {
                        println!("Client disconnected or error.");
                        unsafe { libc::close(pfd.fd) };
                        self.players.remove(&pfd.fd);
                    }
                }
            }
        }
    }

    fn accept_connections(&mut self) -> io::Result<()> {
        loop {
            match self.listener.accept() {
                Ok((stream, addr)) => {
                    println!("Accepted connection from: {}", addr);
                    stream.set_nonblocking(true)?;

                    let client_fd = stream.as_raw_fd();
                    self.players.insert(client_fd, self.game.add_players());

                    std::mem::forget(stream);
                }
                Err(ref e) if e.kind() == io::ErrorKind::WouldBlock => break,
                Err(e) => return Err(e),
            }
        }
        Ok(())
    }

    fn handle_client_data(&mut self, fd: RawFd) -> io::Result<()> {
        let mut buf = [0u8; 512];
        let mut stream = unsafe { std::net::TcpStream::from_raw_fd(fd) };

        loop {
            let mut set_team: bool = false;
            match stream.read(&mut buf) {
                Ok(0) => {
                    println!("Client disconnected.");
                    unsafe { libc::close(fd) };
                    self.players.remove(&fd);
                    break;
                }
                Ok(n) => {
                    let str: String = unsafe { String::from_utf8_unchecked(buf[..n].to_vec()) };
                    match self.players.get_mut(&fd) {
                        Some(e) => {
                            let entity_option = self.game.get_entity(*e);
                            if entity_option.is_none() {
                                continue;
                            }
                            let mut entity = entity_option.unwrap();
                            for i in str.split('\n') {
                                if i.is_empty() {
                                    continue;
                                }
                                match data::parse(i) {
                                    Ok(ac) => {
                                        // TODO: add action ot the actual player
                                        if entity.add_action(ac) {
                                            let _ = stream.write_all("ok".as_bytes());
                                        } else {
                                            let _ = stream.write_all("ko".as_bytes());
                                        }
                                    }
                                    Err(_) => {
                                        let _ = stream.write_all("ko".as_bytes());
                                    }
                                }
                            }
                        }
                        None => {
                            let mut e = Entity::new_dummy();
                            e.set_team(&str);
                            return Ok(());
                        }
                    }
                }
                Err(ref e) if e.kind() == io::ErrorKind::WouldBlock => break,
                Err(e) => {
                    eprintln!("Error reading: {}", e);
                    unsafe { libc::close(fd) };
                    self.players.remove(&fd);
                    break;
                }
            }
        }

        std::mem::forget(stream);
        Ok(())
    }
}

#[cfg(test)]
impl Server {
    pub fn tickrate(&self) -> usize {
        self.tickrate
    }

    pub fn bound_port(&self) -> u16 {
        self.listener.local_addr().expect("listener address").port()
    }
}
