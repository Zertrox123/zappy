#pragma once

#include "model/Tile.hpp"
#include "model/WorldEffect.hpp"

#include <deque>
#include <string>
#include <unordered_map>
#include <vector>

struct Player
{
    int id = 0;
    int x = 0;
    int y = 0;
    int orientation = 1;
    int level = 1;
    std::string team;
    int inventory[7]{};
};

struct Egg
{
    int id = 0;
    int playerId = 0;
    int x = 0;
    int y = 0;
};

class GameState
{
  public:
    int width = 0;
    int height = 0;
    int timeUnit = 100;

    const std::vector<std::vector<Tile>> &tiles() const { return _tiles; }
    const std::unordered_map<int, Player> &players() const { return _players; }
    const std::unordered_map<int, Egg> &eggs() const { return _eggs; }
    const std::vector<std::string> &teams() const { return _teams; }
    const std::string &winner() const { return _winner; }
    const std::vector<WorldEffect> &effects() const { return _effects; }
    const std::deque<std::string> &serverMessages() const
    {
        return _serverMessages;
    }

    int knownTileCount() const { return _knownTileCount; }
    bool isPaused() const { return _paused; }

    bool isGameOver() const { return !_winner.empty(); }

    void resize(int width, int height);
    const Tile &tileAt(int x, int y) const;
    void tickEffects(float deltaSeconds);
    void clearIncantationsAt(int x, int y);

    void addTeam(const std::string &team);
    void setTile(int x, int y, const Tile &tile);
    void noteTileKnown();
    void resetKnownTiles();
    void setPlayer(const Player &player);
    Player &playerOrCreate(int id);
    const Player *findPlayer(int id) const;
    void setPlayerLevel(int id, int level);
    void removePlayer(int id);
    void setEgg(const Egg &egg);
    void removeEgg(int id);
    void setWinner(std::string winner);
    void pushEffect(WorldEffect effect);
    void pushServerMessage(std::string message);
    void setPaused(bool paused);

  private:
    std::vector<std::vector<Tile>> _tiles;
    std::unordered_map<int, Player> _players;
    std::unordered_map<int, Egg> _eggs;
    std::vector<std::string> _teams;
    std::string _winner;
    std::vector<WorldEffect> _effects;
    std::deque<std::string> _serverMessages;
    int _knownTileCount = 0;
    bool _paused = false;
    static constexpr std::size_t kMaxServerMessages = 6;
    static Tile _emptyTile;
};
