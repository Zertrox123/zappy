#include "model/GameState.hpp"
#include "protocol/ProtocolParser.hpp"
#include "render/PlayerAnimator.hpp"
#include "render/Selection.hpp"

#include <cstdlib>
#include <iostream>

namespace
{
bool expect(bool condition, const char *message)
{
    if (!condition)
    {
        std::cerr << "[FAIL] " << message << '\n';
        return false;
    }
    return true;
}
} // namespace

int main()
{
    ProtocolParser parser;
    GameState state;
    state.resize(5, 5);
    parser.parseLine("pnw #1 1 2 2 1 1 name1", state);
    parser.parseLine("pnw #2 2 3 3 2 2 name2", state);

    PlayerAnimator animator;
    animator.reset();
    animator.update(state, 0.f);

    Selection tile = pickSelection(state, animator, 1, 1);
    if (!expect(tile.kind == Selection::Kind::Tile, "empty tile must select tile"))
        return EXIT_FAILURE;
    if (!expect(tile.tileX == 1 && tile.tileY == 1, "tile coords must match click"))
        return EXIT_FAILURE;

    Selection player = pickSelection(state, animator, 2, 2);
    if (!expect(player.kind == Selection::Kind::Player,
                 "occupied tile must select player"))
        return EXIT_FAILURE;
    if (!expect(player.playerId == 1, "player id must match occupant"))
        return EXIT_FAILURE;

    parser.parseLine("ppo #1 1 3 2 3", state);
    animator.update(state, 0.f);
    Selection moving = pickSelection(state, animator, 2, 2);
    if (!expect(moving.kind == Selection::Kind::Player,
                 "animator position must be used for pick"))
        return EXIT_FAILURE;
    if (!expect(moving.playerId == 1,
                 "animator lag must keep player on old tile during move"))
        return EXIT_FAILURE;

    std::cout << "[OK] selection tests passed\n";
    return EXIT_SUCCESS;
}
