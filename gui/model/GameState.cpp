#include "model/GameState.hpp"

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

void GameState::addTeam(const std::string &team) { _teams.push_back(team); }

void GameState::setTile(int x, int y, const Tile &tile)
{
    if (y < 0 || x < 0 || y >= height || x >= width)
        return;
    _tiles[static_cast<std::size_t>(y)][static_cast<std::size_t>(x)] = tile;
}

void GameState::setPlayer(const Player &player)
{
    _players[player.id] = player;
}

Player &GameState::playerOrCreate(int id) { return _players[id]; }

void GameState::setPlayerLevel(int id, int level)
{
    if (_players.count(id))
        _players[id].level = level;
}

void GameState::removePlayer(int id) { _players.erase(id); }

void GameState::setEgg(const Egg &egg) { _eggs[egg.id] = egg; }

void GameState::removeEgg(int id) { _eggs.erase(id); }

void GameState::setWinner(std::string winner) { _winner = std::move(winner); }
