use std::collections::HashMap;
use std::io::{self, Read, Write};
use std::net::TcpListener;
use std::os::unix::io::{AsRawFd, FromRawFd, RawFd};
use std::time::{Duration, SystemTime};

use libc::{EOF, printf};

use crate::data::{Entity, Map, parse};
use crate::game::{Game};

#[derive(Debug)]
pub struct Server {
    listener: TcpListener,
    players: HashMap<RawFd, usize>,
    time: SystemTime,
    tickrate: usize,
    game: Game,
}

impl Server {
    pub fn new(addr: &str, tickrate: usize) -> io::Result<Server> {
        let listener = TcpListener::bind(addr)?;
        listener.set_nonblocking(true)?;

        Ok(Server {
            listener,
            players: HashMap::new(),
            time: SystemTime::now(),
            tickrate,
            game: Game::new()
        })
    }

    pub fn run(&mut self) -> io::Result<()> {
        println!("Server listening...");

        loop {
            let timestamp = SystemTime::now();
            if timestamp.duration_since(self.time).unwrap() > Duration::from_millis((1000 / self.tickrate) as u64) {
                self.time = timestamp;
                self.game.run_ticks();
            }

            // Build the list of file descriptors to poll
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

            // Wait for events with a 1ms timeout
            let n_events = unsafe { libc::poll(fds.as_mut_ptr(), fds.len() as libc::nfds_t, 1) };

            if n_events < 0 {
                return Err(io::Error::last_os_error());
            }

            // Process events
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
                    self.players.insert(client_fd, 0);

                    // Prevent the stream from closing the FD when dropped
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
                    println!("Read bytes");

                    // Fixed: only take the bytes that were actually read
                    let mut str: String = unsafe { String::from_utf8_unchecked(buf[..n].to_vec()) };
                    match self.players.get_mut(&fd) {
                        Some(mut e) => {
                            str.push(' ');
                            str = str.replace('\0', "");
                            for i in str.split('\n') {
                                match parse(i) {
                                    Ok(ac) => {
                                        // TODO: add action ot the actual player
                                        //e.add_action(ac);
                                    }
                                    Err(_) => {}
                                }
                            }
                        }
                        None => {
                            let mut e = Entity::new_dummy();
                            e.set_team(&str);
                            //self.players.insert(fd, e);
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
        
        // Prevent the stream from closing the FD when dropped
        std::mem::forget(stream);
        Ok(())
    }
}
