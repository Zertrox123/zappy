#include "render/PlayerAnimator.hpp"

#include <algorithm>
#include <cmath>

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
            entry.toX = static_cast<float>(player.x);
            entry.toY = static_cast<float>(player.y);
            entry.elapsed = 0.f;
        }

        if (entry.elapsed < entry.duration)
        {
            entry.elapsed =
                std::min(entry.elapsed + deltaSeconds, entry.duration);
            const float t = entry.elapsed / entry.duration;
            entry.x = entry.fromX + (entry.toX - entry.fromX) * t;
            entry.y = entry.fromY + (entry.toY - entry.fromY) * t;
        }
        else
        {
            entry.x = entry.toX;
            entry.y = entry.toY;
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
