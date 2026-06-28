#pragma once

#include "model/GameState.hpp"
#include "net/NetworkClient.hpp"
#include "net/ReceiveBuffer.hpp"
#include "protocol/GuiRequests.hpp"
#include "protocol/ProtocolParser.hpp"
#include "render/EffectRenderer.hpp"
#include "render/MapCamera.hpp"
#include "render/PanInput.hpp"
#include "render/PlayerAnimator.hpp"
#include "render/Selection.hpp"
#include "render/Sidebar.hpp"
#include "render/ZoomInput.hpp"

#include <SFML/Graphics.hpp>
#include <string>

class GuiWindow
{
  public:
    GuiWindow(NetworkClient &client, ReceiveBuffer &buffer,
              const std::string &host, int port);

    int run();

  private:
    static constexpr unsigned kDefaultMapWidth = 1280;
    static constexpr unsigned kDefaultMapHeight = 800;
    static constexpr unsigned kMinWindowWidth = 960;
    static constexpr unsigned kMinWindowHeight = 600;
    static constexpr float kPanSpeedTilesPerSec = 10.f;

    NetworkClient &_client;
    ReceiveBuffer &_buffer;
    ProtocolParser _parser;
    GameState _state;
    PlayerAnimator _animator;
    EffectRenderer _effects;
    Sidebar _sidebar;
    MapCamera _camera;
    sf::View _mapView;
    Selection _selection;
    unsigned _mapPixelWidth = 0;
    unsigned _mapPixelHeight = 0;
    PanInputState _panInput;
    bool _zoomHandledThisFrame = false;
    std::string _host;
    int _port;

    void bootstrapState();
    void pullNetwork();
    void updateLayout(sf::RenderWindow &window);
    void refreshMapView(sf::RenderWindow &window);
    void updatePan(float deltaSeconds);
    void handleKeyPress(sf::Keyboard::Key key, sf::Keyboard::Scancode scancode);
    void handleKeyRelease(sf::Keyboard::Key key,
                          sf::Keyboard::Scancode scancode);
    bool handleZoomKey(sf::Keyboard::Key key, sf::Keyboard::Scancode scancode);
    bool handleZoomText(char32_t character);
    bool handleProtocolKey(sf::Keyboard::Key key);
    void handleClick(const sf::RenderWindow &window, int pixelX, int pixelY);
    sf::Color tileColor(int x, int y) const;
    sf::Color teamColor(const std::string &team) const;
    void drawMap(sf::RenderWindow &window) const;
    void drawResources(sf::RenderWindow &window) const;
    void drawEggs(sf::RenderWindow &window) const;
    void drawPlayers(sf::RenderWindow &window);
    void drawSelection(sf::RenderWindow &window) const;
    void drawGameOver(sf::RenderWindow &window, unsigned mapPixelWidth) const;
    void drawPause(sf::RenderWindow &window, unsigned mapPixelWidth) const;
};
