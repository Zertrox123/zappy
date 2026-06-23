#pragma once

#include <string>
#include <vector>

enum class EffectKind
{
    Expulsion,
    Broadcast,
    Incantation,
};

struct WorldEffect
{
    EffectKind kind = EffectKind::Expulsion;
    int x = 0;
    int y = 0;
    int playerId = 0;
    int level = 1;
    std::string message;
    std::vector<int> participants;
    float age = 0.f;
};
