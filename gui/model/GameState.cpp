#include "model/GameState.hpp"

#include <sstream>

Tile GameState::_emptyTile{};

void GameState::resize(int width, int height)
{
    this->width = width;
    this->height = height;
    _tiles.assign(static_cast<std::size_t>(height),
                  std::vector<Tile>(static_cast<std::size_t>(width)));
}

const Tile &GameState::tileAt(int x, int y) const
{
    if (y < 0 || x < 0 || y >= height || x >= width)
        return _emptyTile;
    return _tiles[static_cast<std::size_t>(y)][static_cast<std::size_t>(x)];
}

bool GameState::readResources(std::istringstream &iss, int (&out)[7])
{
    for (int i = 0; i < 7; ++i)
    {
        if (!(iss >> out[i]))
            return false;
    }
    return true;
}

void GameState::applyLine(const std::string &line)
{
    if (line.empty())
        return;

    std::istringstream iss(line);
    std::string cmd;
    iss >> cmd;

    if (cmd == "msz")
    {
        int w = 0;
        int h = 0;
        iss >> w >> h;
        resize(w, h);
        return;
    }

    if (cmd == "pnw")
    {
        Player player;
        std::string playerNum;
        iss >> playerNum >> player.id >> player.x >> player.y >>
            player.orientation >> player.level >> player.team;
        _players[player.id] = player;
        return;
    }

    if (cmd == "ppo")
    {
        int id = 0;
        std::string playerNum;
        iss >> playerNum >> id;
        Player &player = _players[id];
        player.id = id;
        iss >> player.x >> player.y >> player.orientation;
        return;
    }

    if (cmd == "plv")
    {
        int id = 0;
        int level = 0;
        std::string playerNum;
        iss >> playerNum >> id >> level;
        if (_players.count(id))
            _players[id].level = level;
        return;
    }

    if (cmd == "pdi")
    {
        int id = 0;
        std::string playerNum;
        iss >> playerNum >> id;
        _players.erase(id);
        return;
    }
}
