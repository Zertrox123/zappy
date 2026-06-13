use crate::data::{Map, Resource, ResourceCounts};
use crate::game::Game;

#[test]
fn populate_reaches_density_targets_on_10_by_10_map() {
    let mut map = Map::new(10, 10);
    map.populate();

    assert_eq!(
        map.resource_counts(),
        ResourceCounts {
            food: 50,
            linemate: 30,
            deraumere: 15,
            sibur: 10,
            mendiane: 10,
            phiras: 8,
            thystame: 5,
        }
    );
}

#[test]
fn refill_restores_missing_resources_up_to_target() {
    let mut map = Map::new(10, 10);
    map.populate();

    let removed = map.deplete(Resource::Food, 12);
    assert_eq!(removed, 12);
    assert_eq!(map.resource_counts().food, 38);

    map.refill();

    assert_eq!(map.resource_counts(), map.target_counts());
}

#[test]
fn refill_does_not_exceed_target_when_map_is_full() {
    let mut map = Map::new(10, 10);
    map.populate();
    let before = map.resource_counts();

    map.refill();

    assert_eq!(map.resource_counts(), before);
}

#[test]
fn game_refills_every_20_ticks_not_before() {
    let mut game = Game::new(10, 10, vec!["team".to_string()], 5);
    let removed = game.deplete(Resource::Linemate, 7);
    assert_eq!(removed, 7);
    assert_eq!(game.resource_counts().linemate, 23);

    for _ in 0..19 {
        game.run_ticks();
    }
    assert_eq!(game.resource_counts().linemate, 23);

    game.run_ticks();
    assert_eq!(game.resource_counts().linemate, 30);
}

#[test]
fn game_refill_restores_full_deficit_in_one_cycle() {
    let mut game = Game::new(10, 10, vec!["team".to_string()], 5);
    game.deplete(Resource::Food, 25);

    for _ in 0..20 {
        game.run_ticks();
    }

    assert_eq!(game.resource_counts(), game.target_resource_counts());
}
