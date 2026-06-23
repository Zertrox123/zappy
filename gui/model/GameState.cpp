#include "model/GameState.hpp"

#include <algorithm>

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

void GameState::pushEffect(WorldEffect effect)
{
    _effects.push_back(std::move(effect));
}

void GameState::tickEffects(float deltaSeconds)
{
    for (WorldEffect &effect : _effects)
        effect.age += deltaSeconds;

    _effects.erase(std::remove_if(_effects.begin(), _effects.end(),
                                  [](const WorldEffect &effect)
                                  {
                                      switch (effect.kind)
                                      {
                                      case EffectKind::Expulsion:
                                          return effect.age > 0.6f;
                                      case EffectKind::Broadcast:
                                          return effect.age > 3.f;
                                      case EffectKind::Incantation:
                                          return effect.age > 8.f;
                                      }
                                      return true;
                                  }),
                   _effects.end());
}

void GameState::clearIncantationsAt(int x, int y)
{
    _effects.erase(std::remove_if(_effects.begin(), _effects.end(),
                                  [x, y](const WorldEffect &effect)
                                  {
                                      return effect.kind ==
                                                 EffectKind::Incantation &&
                                             effect.x == x && effect.y == y;
                                  }),
                   _effects.end());
}

void GameState::addTeam(const std::string &team) { _teams.push_back(team); }

void GameState::setTile(int x, int y, const Tile &tile)
{
    if (y < 0 || x < 0 || y >= height || x >= width)
        return;
    _tiles[static_cast<std::size_t>(y)][static_cast<std::size_t>(x)] = tile;
}

void GameState::setPlayer(const Player &player) { _players[player.id] = player; }

Player &GameState::playerOrCreate(int id) { return _players[id]; }

const Player *GameState::findPlayer(int id) const
{
    const auto it = _players.find(id);
    if (it == _players.end())
        return nullptr;
    return &it->second;
}

void GameState::setPlayerLevel(int id, int level)
{
    if (_players.count(id))
        _players[id].level = level;
}

void GameState::removePlayer(int id) { _players.erase(id); }

void GameState::setEgg(const Egg &egg) { _eggs[egg.id] = egg; }

void GameState::removeEgg(int id) { _eggs.erase(id); }

void GameState::setWinner(std::string winner) { _winner = std::move(winner); }
