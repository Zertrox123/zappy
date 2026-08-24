# Zappy

Distributed multiplayer network game: TCP server in C, autonomous AI in Python/C++, and visualizer.

## Overview

Simulates a real-time world where teams of AI drones compete for resources and perform elevation rituals through sound broadcasts and distributed coordination.

## Getting Started

```bash
# Build all components
make

# Run server: ./zappy_server -p <port> -x <width> -y <height> -n <team1> <team2> -c <clientsNb> -t <freq>
./zappy_server -p 4242 -x 20 -y 20 -n Team1 Team2 -c 6 -t 100

# Run AI client
./zappy_ai -p 4242 -n Team1 -h 127.0.0.1
```
