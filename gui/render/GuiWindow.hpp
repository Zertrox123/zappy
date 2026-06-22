#pragma once

#include "model/GameState.hpp"
#include "net/NetworkClient.hpp"
#include "net/ReceiveBuffer.hpp"
#include "protocol/ProtocolParser.hpp"

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
    std::string _host;
    int _port;

    void bootstrapState();
    void pullNetwork();
    sf::Color tileColor(int x, int y) const;
    sf::Color teamColor(const std::string &team) const;
    void drawPlayers(sf::RenderWindow &window) const;
};
