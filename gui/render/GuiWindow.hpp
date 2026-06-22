#pragma once

#include "model/GameState.hpp"
#include "net/NetworkClient.hpp"
#include "net/ReceiveBuffer.hpp"
#include "protocol/ProtocolParser.hpp"
#include "render/PlayerAnimator.hpp"

#include <SFML/Graphics.hpp>
#include <string>

class GuiWindow
{
  public:
    GuiWindow(NetworkClient &client, ReceiveBuffer &buffer,
              const std::string &host, int port);

    int run();

  private:
    static constexpr unsigned kTileSize = 32;
    static constexpr unsigned kMaxWindow = 960;

    NetworkClient &_client;
    ReceiveBuffer &_buffer;
    ProtocolParser _parser;
    GameState _state;
    PlayerAnimator _animator;
    std::string _host;
    int _port;

    void bootstrapState();
    void pullNetwork();
    sf::Color tileColor(int x, int y) const;
    sf::Color teamColor(const std::string &team) const;
    void drawResources(sf::RenderWindow &window) const;
    void drawEggs(sf::RenderWindow &window) const;
    void drawPlayers(sf::RenderWindow &window);
    void drawGameOver(sf::RenderWindow &window) const;
};
