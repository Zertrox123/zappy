#include "model/GameState.hpp"
#include "protocol/ProtocolParser.hpp"

#include <cstdlib>
#include <iostream>
#include <string>

namespace
{
bool expect(bool condition, const char *message)
{
    if (!condition)
    {
        std::cerr << "[test_protocol_parser] " << message << '\n';
        return false;
    }
    return true;
}

void feed(ProtocolParser &parser, GameState &state, const std::string &payload)
{
    std::size_t pos = 0;
    while (pos < payload.size())
    {
        const std::size_t end = payload.find('\n', pos);
        const std::size_t lineEnd = end == std::string::npos ? payload.size() : end;
        parser.parseLine(payload.substr(pos, lineEnd - pos), state);
        if (end == std::string::npos)
            break;
        pos = end + 1;
    }
}
} // namespace

int main()
{
    ProtocolParser parser;
    GameState state;
    feed(parser, state,
         "msz 3 2\n"
         "sgt 100\n"
         "tna team1\n"
         "bct 1 0 2 0 1 0 0 0 0\n"
         "pnw #1 1 1 0 1 1 team1\n"
         "ppo #1 1 2 0 2\n"
         "pin #1 1 2 0 3 0 1 0 0 0 0\n"
         "enw #0 #1 0 1 1\n"
         "edi #0\n");

    if (!expect(state.width == 3 && state.height == 2, "msz must resize map"))
        return EXIT_FAILURE;
    if (!expect(state.timeUnit == 100, "sgt must set time unit"))
        return EXIT_FAILURE;
    if (!expect(state.teams().size() == 1 && state.teams()[0] == "team1",
                "tna must register team"))
        return EXIT_FAILURE;
    if (!expect(state.tileAt(1, 0).resources[0] == 2, "bct must set tile resources"))
        return EXIT_FAILURE;
    if (!expect(state.players().count(1) == 1, "pnw must create player"))
        return EXIT_FAILURE;

    const Player &player = state.players().at(1);
    if (!expect(player.x == 2 && player.y == 0 && player.orientation == 2,
                "ppo must update player position"))
        return EXIT_FAILURE;
    if (!expect(player.inventory[0] == 3, "pin must update inventory"))
        return EXIT_FAILURE;
    if (!expect(state.eggs().empty(), "edi must remove egg"))
        return EXIT_FAILURE;

    parser.parseLine("seg team1", state);
    if (!expect(state.isGameOver() && state.winner() == "team1",
                "seg must set winner"))
        return EXIT_FAILURE;

    parser.parseLine("enw #2 #-1 4 5", state);
    if (!expect(state.eggs().count(2) == 1, "enw must register egg"))
        return EXIT_FAILURE;
    if (!expect(state.eggs().at(2).playerId == -1,
                "enw must accept #-1 player id"))
        return EXIT_FAILURE;

    state.resetKnownTiles();
    parser.parseLine("bct 0 0 1 0 0 0 0 0 0", state);
    parser.parseLine("bct 0 0 2 0 0 0 0 0 0", state);
    if (!expect(state.knownTileCount() == 1,
                "duplicate bct must not inflate known tile count"))
        return EXIT_FAILURE;

    parser.parseLine("sst 250", state);
    if (!expect(state.timeUnit == 250, "sst must update time unit"))
        return EXIT_FAILURE;

    parser.parseLine("suc", state);
    if (!expect(!state.serverMessages().empty(), "suc must surface server message"))
        return EXIT_FAILURE;

    parser.parseLine("sbp", state);
    if (!expect(state.serverMessages().size() >= 2,
                "sbp must surface server message"))
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}
