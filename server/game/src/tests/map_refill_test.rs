use crate::data::{Map, Resource, RESOURCES};
use crate::game::Game;

fn assert_at_target(map: &Map) {
    for resource in RESOURCES {
        assert_eq!(map.count(resource), map.max_resources(resource));
    }
}

#[test]
fn populate_reaches_density_targets_on_10_by_10_map() {
    let mut map = Map::new(10, 10);
    map.populate();

    assert_eq!(map.count(Resource::Food), 50);
    assert_eq!(map.count(Resource::Linemate), 30);
    assert_eq!(map.count(Resource::Deraumere), 15);
    assert_eq!(map.count(Resource::Sibur), 10);
    assert_eq!(map.count(Resource::Mendiane), 10);
    assert_eq!(map.count(Resource::Phiras), 8);
    assert_eq!(map.count(Resource::Thystame), 5);
}

#[test]
fn refill_restores_missing_resources_up_to_target() {
    let mut map = Map::new(10, 10);
    map.populate();

    let removed = map.deplete(Resource::Food, 12);
    assert_eq!(removed, 12);
    assert_eq!(map.count(Resource::Food), 38);

    map.refill();

    assert_at_target(&map);
}

#[test]
fn refill_does_not_exceed_target_when_map_is_full() {
    let mut map = Map::new(10, 10);
    map.populate();
    assert_at_target(&map);

    map.refill();

    assert_at_target(&map);
}

#[test]
fn game_refills_every_20_ticks_not_before() {
    let mut game = Game::new(10, 10, vec!["team".to_string()], 5);
    let removed = game.deplete(Resource::Linemate, 7);
    assert_eq!(removed, 7);
    assert_eq!(game.count(Resource::Linemate), 23);

    for _ in 0..19 {
        game.run_ticks();
    }
    assert_eq!(game.count(Resource::Linemate), 23);

    game.run_ticks();
    assert_eq!(game.count(Resource::Linemate), 30);
}

#[test]
fn game_refill_restores_full_deficit_in_one_cycle() {
    let mut game = Game::new(10, 10, vec!["team".to_string()], 5);
    game.deplete(Resource::Food, 25);

    for _ in 0..20 {
        game.run_ticks();
    }

    assert_eq!(game.count(Resource::Food), 50);
}
