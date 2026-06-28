#include "render/PlayerAnimator.hpp"

#include <algorithm>
#include <cmath>

namespace
{
float wrapDelta(float from, float to, int size)
{
    if (size <= 1)
        return to - from;
    float delta = to - from;
    const float limit = static_cast<float>(size) / 2.f;
    while (delta > limit)
        delta -= static_cast<float>(size);
    while (delta < -limit)
        delta += static_cast<float>(size);
    return delta;
}

float normalizeCoord(float value, int size)
{
    if (size <= 0)
        return value;
    float result = std::fmod(value, static_cast<float>(size));
    if (result < 0.f)
        result += static_cast<float>(size);
    return result;
}
} // namespace

void PlayerAnimator::reset() { _entries.clear(); }

float PlayerAnimator::moveDuration(const GameState &state) const
{
    const float freq = static_cast<float>(std::max(1, state.timeUnit));
    return 7.f / freq;
}

void PlayerAnimator::update(const GameState &state, float deltaSeconds)
{
    const float duration = moveDuration(state);

    for (const auto &[id, player] : state.players())
    {
        Entry &entry = _entries[id];
        entry.orientation = player.orientation;
        entry.level = player.level;
        entry.team = player.team;
        entry.duration = duration;

        if (!entry.initialized)
        {
            entry.x = static_cast<float>(player.x);
            entry.y = static_cast<float>(player.y);
            entry.toX = entry.x;
            entry.toY = entry.y;
            entry.initialized = true;
            continue;
        }

        if (player.x != static_cast<int>(entry.toX) ||
            player.y != static_cast<int>(entry.toY))
        {
            entry.fromX = entry.x;
            entry.fromY = entry.y;
            entry.toX = entry.fromX + wrapDelta(entry.fromX,
                                                static_cast<float>(player.x),
                                                state.width);
            entry.toY = entry.fromY + wrapDelta(entry.fromY,
                                                static_cast<float>(player.y),
                                                state.height);
            entry.elapsed = 0.f;
        }

        if (entry.elapsed < entry.duration)
        {
            entry.elapsed =
                std::min(entry.elapsed + deltaSeconds, entry.duration);
            const float t = entry.elapsed / entry.duration;
            entry.x = normalizeCoord(
                entry.fromX + (entry.toX - entry.fromX) * t, state.width);
            entry.y = normalizeCoord(
                entry.fromY + (entry.toY - entry.fromY) * t, state.height);
        }
        else
        {
            entry.x = normalizeCoord(entry.toX, state.width);
            entry.y = normalizeCoord(entry.toY, state.height);
            entry.toX = entry.x;
            entry.toY = entry.y;
        }
    }

    for (auto it = _entries.begin(); it != _entries.end();)
    {
        if (state.players().count(it->first) == 0)
            it = _entries.erase(it);
        else
            ++it;
    }
}

bool PlayerAnimator::snapshot(int playerId, Snapshot &out) const
{
    const auto it = _entries.find(playerId);
    if (it == _entries.end())
        return false;

    out.x = it->second.x;
    out.y = it->second.y;
    out.orientation = it->second.orientation;
    out.level = it->second.level;
    out.team = it->second.team;
    return true;
}
