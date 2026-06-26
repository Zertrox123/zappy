#include "model/GameState.hpp"

#include <sstream>

void GameState::setMapSize(int width, int height)
{
    this->width = width;
    this->height = height;
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
        iss >> width >> height;
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
    }
}
