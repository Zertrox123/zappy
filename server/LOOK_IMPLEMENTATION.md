# Look implementation proposal

This document describes the proposed implementation of the Zappy `Look` command. No Rust source changes are included here.

## Required behavior

- `Look` completes after 7 ticks.
- A level `L` player sees `(L + 1)^2` tiles.
- Tiles form a forward-facing triangle and are ordered from left to right on each depth line.
- Coordinates wrap around the map.
- Multiple objects on one tile are separated by spaces.
- Tiles are separated by commas.
- The response ends with a newline: `[tile0,tile1,...]\n`.

Players will be detected from `Game::players`, because map tiles do not currently track player movement reliably.

## Data accessors

Add the following methods in `game/src/data.rs`:

```rust
impl Tile {
    pub fn resources(&self) -> &[Resource] {
        &self.stone
    }
}

impl Entity {
    pub fn position(&self) -> Position {
        self.pos
    }

    pub fn direction(&self) -> Direction {
        self.direction
    }

    pub fn level(&self) -> usize {
        self.level
    }
}

impl Resource {
    pub fn name(self) -> &'static str {
        match self {
            Resource::Food => "food",
            Resource::Linemate => "linemate",
            Resource::Deraumere => "deraumere",
            Resource::Sibur => "sibur",
            Resource::Mendiane => "mendiane",
            Resource::Phiras => "phiras",
            Resource::Thystame => "thystame",
        }
    }
}
```

Players should start at level 1:

```rust
Entity {
    id: 0,
    raw_fd: -1,
    team: String::new(),
    saturation: 142,
    level: 1,
    dummy: true,
    pos: Position { x: 0, y: 0 },
    direction: Direction::South,
    actions: Vec::new(),
    reply: String::new(),
}
```

## Vision calculation

Add the following methods in `game/src/game.rs`:

```rust
fn vision_vectors(direction: Direction) -> ((isize, isize), (isize, isize)) {
    match direction {
        Direction::North => ((0, 1), (1, 0)),
        Direction::South => ((0, -1), (-1, 0)),
        Direction::Est => ((1, 0), (0, -1)),
        Direction::West => ((-1, 0), (0, 1)),
        Direction::None => ((0, 0), (0, 0)),
    }
}

fn tile_content(&self, x: isize, y: isize) -> String {
    let (width, height) = self.map.dimensions();
    let wrapped_x = x.rem_euclid(width as isize);
    let wrapped_y = y.rem_euclid(height as isize);
    let mut objects = Vec::new();

    for player in &self.players {
        let pos = player.position();

        if (pos.x as isize).rem_euclid(width as isize) == wrapped_x
            && (pos.y as isize).rem_euclid(height as isize) == wrapped_y
        {
            objects.push("player");
        }
    }

    for resource in self.map.get(wrapped_x, wrapped_y).resources() {
        objects.push(resource.name());
    }

    objects.join(" ")
}

fn look(&self, player_index: usize) -> String {
    let player = &self.players[player_index];
    let position = player.position();
    let (forward, right) = Self::vision_vectors(player.direction());
    let mut tiles = Vec::new();

    for depth in 0..=player.level() as isize {
        for lateral in -depth..=depth {
            let x = position.x as isize
                + forward.0 * depth
                + right.0 * lateral;
            let y = position.y as isize
                + forward.1 * depth
                + right.1 * lateral;

            tiles.push(self.tile_content(x, y));
        }
    }

    format!("[{}]\n", tiles.join(","))
}
```

## Action completion

Refactor `Game::do_action` so `Look` can inspect the game after the mutable player borrow ends:

```rust
pub fn do_action(&mut self) {
    for player_index in 0..self.players.len() {
        let completed_action = {
            let player = &mut self.players[player_index];

            let Some(action) = player.actions.first_mut() else {
                continue;
            };

            action.reduce_timeleft();

            if action.is_complete() {
                Some(player.actions.remove(0).kind())
            } else {
                None
            }
        };

        let Some(action) = completed_action else {
            continue;
        };

        let response = match action {
            EAction::Forward => {
                self.players[player_index].forward();
                "ok\n".to_string()
            }
            EAction::Left => {
                self.players[player_index].rotate(Rotation::Left);
                "ok\n".to_string()
            }
            EAction::Right => {
                self.players[player_index].rotate(Rotation::Right);
                "ok\n".to_string()
            }
            EAction::Look => self.look(player_index),
            _ => "ok\n".to_string(),
        };

        let fd = self.players[player_index].raw_fd();
        self.reply.insert(fd, response);
    }
}
```

## Tests

The implementation should include tests for:

- level-1 tile count and ordering;
- North, South, East, and West orientations;
- horizontal and vertical map wrapping;
- multiple players on one tile;
- multiple resources on one tile;
- the exact response format and trailing newline;
- completion after 7 ticks rather than immediately.

Example response:

```text
[player,player deraumere,,]
```
