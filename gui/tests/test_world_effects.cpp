#include "model/GameState.hpp"
#include "protocol/ProtocolParser.hpp"

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
    parser.parseLine("pnw #1 1 2 2 1 1 team1", state);

    parser.parseLine("pex #1", state);
    if (!expect(state.effects().size() == 1, "pex must create expulsion effect"))
        return EXIT_FAILURE;
    if (!expect(state.effects().front().kind == EffectKind::Expulsion,
                 "pex effect kind must be expulsion"))
        return EXIT_FAILURE;

    parser.parseLine("pbc #1 hello world", state);
    if (!expect(state.effects().size() == 2, "pbc must append broadcast effect"))
        return EXIT_FAILURE;
    if (!expect(state.effects().back().message == "hello world",
                 "pbc must capture full message"))
        return EXIT_FAILURE;

    parser.parseLine("pic 2 2 2 #1 #2", state);
    if (!expect(state.effects().size() == 3, "pic must append incantation effect"))
        return EXIT_FAILURE;
    if (!expect(state.effects().back().participants.size() == 2,
                 "pic must list participants"))
        return EXIT_FAILURE;

    parser.parseLine("pie 2 2 1", state);
    if (!expect(state.effects().size() == 2,
                 "pie must clear incantation at tile"))
        return EXIT_FAILURE;

    state.tickEffects(1.f);
    if (!expect(state.effects().size() == 1,
                 "tick must prune short-lived expulsion effects"))
        return EXIT_FAILURE;

    state.tickEffects(3.f);
    if (!expect(state.effects().empty(), "tick must prune expired effects"))
        return EXIT_FAILURE;

    std::cout << "[OK] world effect tests passed\n";
    return EXIT_SUCCESS;
}
