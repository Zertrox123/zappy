#pragma once

#include <string>
#include <unordered_map>

struct Player
{
    int id = 0;
    int x = 0;
    int y = 0;
    int orientation = 1;
    int level = 1;
    std::string team;
};

class GameState
{
  public:
    int width = 0;
    int height = 0;

    const std::unordered_map<int, Player> &players() const { return _players; }

    void setMapSize(int width, int height);
    void applyLine(const std::string &line);

  private:
    std::unordered_map<int, Player> _players;
};
