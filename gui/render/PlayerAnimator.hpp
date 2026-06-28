#pragma once

#include "model/GameState.hpp"

#include <unordered_map>

class PlayerAnimator
{
  public:
    struct Snapshot
    {
        float x = 0.f;
        float y = 0.f;
        int orientation = 1;
        int level = 1;
        std::string team;
    };

    void reset();
    void update(const GameState &state, float deltaSeconds);
    bool snapshot(int playerId, Snapshot &out) const;

  private:
    struct Entry
    {
        float x = 0.f;
        float y = 0.f;
        float fromX = 0.f;
        float fromY = 0.f;
        float toX = 0.f;
        float toY = 0.f;
        float elapsed = 0.f;
        float duration = 0.07f;
        int orientation = 1;
        int level = 1;
        std::string team;
        bool initialized = false;
    };

    std::unordered_map<int, Entry> _entries;

    float moveDuration(const GameState &state) const;
};
