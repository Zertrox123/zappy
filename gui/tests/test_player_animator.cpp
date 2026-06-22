#include "render/PlayerAnimator.hpp"

#include <cstdlib>
#include <iostream>

namespace
{
bool expect(bool condition, const char *message)
{
    if (!condition)
    {
        std::cerr << "[test_player_animator] " << message << '\n';
        return false;
    }
    return true;
}
} // namespace

int main()
{
    GameState state;
    state.timeUnit = 100;
    state.applyLine("msz 5 5");
    state.applyLine("pnw #1 1 0 0 1 1 team1");

    PlayerAnimator animator;
    PlayerAnimator::Snapshot snap{};
    animator.update(state, 0.f);

    if (!expect(animator.snapshot(1, snap), "player snapshot must exist"))
        return EXIT_FAILURE;
    if (!expect(snap.x == 0.f && snap.y == 0.f, "initial position must match"))
        return EXIT_FAILURE;

    state.applyLine("ppo #1 1 2 0 2");
    animator.update(state, 0.f);
    if (!expect(animator.snapshot(1, snap), "snapshot after ppo"))
        return EXIT_FAILURE;
    if (!expect(snap.x == 0.f && snap.y == 0.f,
                "animation must start at old position"))
        return EXIT_FAILURE;

    animator.update(state, 0.035f);
    if (!expect(animator.snapshot(1, snap), "snapshot mid-animation"))
        return EXIT_FAILURE;
    if (!expect(snap.x > 0.f && snap.x < 2.f,
                "player must be between tiles mid-animation"))
        return EXIT_FAILURE;

    animator.update(state, 0.07f);
    if (!expect(animator.snapshot(1, snap), "snapshot after animation"))
        return EXIT_FAILURE;
    if (!expect(snap.x == 2.f && snap.y == 0.f,
                "animation must finish on target tile"))
        return EXIT_FAILURE;

    state.applyLine("pdi #1 1");
    animator.update(state, 0.f);
    if (!expect(!animator.snapshot(1, snap), "removed player must disappear"))
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}
