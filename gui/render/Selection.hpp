#pragma once

#include "model/GameState.hpp"
#include "net/NetworkClient.hpp"
#include "render/PlayerAnimator.hpp"

struct Selection
{
    enum class Kind
    {
        None,
        Tile,
        Player,
    };

    Kind kind = Kind::None;
    int tileX = 0;
    int tileY = 0;
    int playerId = 0;
};

Selection pickSelection(const GameState &state, const PlayerAnimator &animator,
                        int tileX, int tileY);

void requestSelectionRefresh(NetworkClient &client, const Selection &selection);
