#include "render/Selection.hpp"

#include <sstream>

Selection pickSelection(const GameState &state, const PlayerAnimator &animator,
                        int tileX, int tileY)
{
    Selection selection{};
    if (tileX < 0 || tileY < 0 || tileX >= state.width || tileY >= state.height)
        return selection;

    PlayerAnimator::Snapshot snap{};
    for (const auto &[id, player] : state.players())
    {
        (void)player;
        if (!animator.snapshot(id, snap))
            continue;
        const int px = static_cast<int>(snap.x + 0.5f);
        const int py = static_cast<int>(snap.y + 0.5f);
        if (px == tileX && py == tileY)
        {
            selection.kind = Selection::Kind::Player;
            selection.playerId = id;
            selection.tileX = tileX;
            selection.tileY = tileY;
            return selection;
        }
    }

    selection.kind = Selection::Kind::Tile;
    selection.tileX = tileX;
    selection.tileY = tileY;
    return selection;
}

void requestSelectionRefresh(NetworkClient &client, const Selection &selection)
{
    if (!client.isConnected())
        return;

    std::ostringstream out;
    if (selection.kind == Selection::Kind::Player)
        out << "pin #" << selection.playerId << '\n';
    else if (selection.kind == Selection::Kind::Tile)
        out << "bct " << selection.tileX << ' ' << selection.tileY << '\n';
    else
        return;

    client.sendRaw(out.str());
}
