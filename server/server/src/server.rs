use std::collections::{HashMap, HashSet};
use std::io::{self, Read, Write};
use std::net::TcpListener;
use std::os::unix::io::{AsRawFd, FromRawFd, RawFd};
use std::time::{Duration, SystemTime};

use crate::config::ServerConfig;

pub struct ClientReply {
    pub data: Vec<u8>,
    pub disconnect: bool,
}

impl ClientReply {
    pub fn data(data: Vec<u8>) -> Self {
        Self {
            data,
            disconnect: false,
        }
    }

    pub fn data_then_close(data: Vec<u8>) -> Self {
        Self {
            data,
            disconnect: true,
        }
    }
}

pub trait ClientHandler {
    fn tick(&mut self) -> HashMap<i32, String>;
    fn on_connect(&mut self, client_fd: u64) -> Vec<u8>;
    fn client_message(&mut self, client_fd: u64, data: &str) -> Option<ClientReply>;
    fn client_disconnect(&mut self, client_fd: u64);
}

pub struct Server<H: ClientHandler> {
    listener: TcpListener,
    clients: HashSet<RawFd>,
    time: SystemTime,
    tickrate: usize,
    handler: H,
}

impl<H: ClientHandler> Server<H> {
    pub fn new(config: &ServerConfig, handler: H) -> io::Result<Self> {
        let addr = format!("0.0.0.0:{}", config.port);
        let listener = TcpListener::bind(&addr)?;
        listener.set_nonblocking(true)?;

        Ok(Server {
            listener,
            clients: HashSet::new(),
            time: SystemTime::now(),
            tickrate: config.frequency as usize,
            handler,
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
                for (i, v) in self.handler.tick() {
                    let Some(fd) = self.clients.iter().find(|&&fd| fd == i).copied() else {
                        continue;
                    };
                    let mut stream = unsafe { std::net::TcpStream::from_raw_fd(fd) };
                    let _ = stream.write_all(v.as_bytes());
                    std::mem::forget(stream);
                }
            }

            let mut fds: Vec<libc::pollfd> = Vec::new();
            fds.push(libc::pollfd {
                fd: self.listener.as_raw_fd(),
                events: libc::POLLIN,
                revents: 0,
            });

            for &fd in &self.clients {
                fds.push(libc::pollfd {
                    fd,
                    events: libc::POLLIN,
                    revents: 0,
                });
            }

            let n_events = unsafe {
                libc::poll(
                    fds.as_mut_ptr(),
                    fds.len() as libc::nfds_t,
                    1000 / self.tickrate as i32,
                )
            };

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
                        self.disconnect_client(pfd.fd);
                    }
                }
            }
        }
    }

    fn accept_connections(&mut self) -> io::Result<()> {
        loop {
            match self.listener.accept() {
                Ok((mut stream, addr)) => {
                    println!("Accepted connection from: {}", addr);

                    let client_fd = stream.as_raw_fd();
                    let client_id = client_fd as u64;
                    let welcome = self.handler.on_connect(client_id);
                    if !welcome.is_empty() {
                        let _ = stream.write_all(&welcome);
                    }
                    stream.set_nonblocking(true)?;
                    self.clients.insert(client_fd);

                    std::mem::forget(stream);
                }
                Err(ref e) if e.kind() == io::ErrorKind::WouldBlock => break,
                Err(e) => return Err(e),
            }
        }
        Ok(())
    }

    fn disconnect_client(&mut self, fd: RawFd) {
        if self.clients.remove(&fd) {
            self.handler.client_disconnect(fd as u64);
        }
        unsafe { libc::close(fd) };
    }

    fn handle_client_data(&mut self, fd: RawFd) -> io::Result<()> {
        let mut buf = [0u8; 512];
        let mut stream = unsafe { std::net::TcpStream::from_raw_fd(fd) };

        loop {
            match stream.read(&mut buf) {
                Ok(0) => {
                    println!("Client disconnected.");
                    self.disconnect_client(fd);
                    break;
                }
                Ok(n) => {
                    let data = String::from_utf8_lossy(&buf[..n]).to_string();
                    if self.clients.contains(&fd) {
                        for line in data.split('\n') {
                            if line.is_empty() {
                                continue;
                            }
                            let reply = self.handler.client_message(fd as u64, line);
                            if reply.is_none() {
                                continue;
                            }
                            let repl = reply.expect("client_message should never return None");
                            if !repl.data.is_empty() {
                                let _ = stream.write_all(&repl.data);
                            }
                            if repl.disconnect {
                                self.disconnect_client(fd);
                                break;
                            }
                        }
                    }
                }
                Err(ref e) if e.kind() == io::ErrorKind::WouldBlock => break,
                Err(e) => {
                    eprintln!("Error reading: {}", e);
                    self.disconnect_client(fd);
                    break;
                }
            }
        }

        std::mem::forget(stream);
        Ok(())
    }
}

#[cfg(test)]
impl<H: ClientHandler> Server<H> {
    pub fn tickrate(&self) -> usize {
        self.tickrate
    }

    pub fn bound_port(&self) -> u16 {
        self.listener.local_addr().expect("listener address").port()
    }
}
