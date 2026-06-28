#include "render/GuiWindow.hpp"
#include "protocol/MapSync.hpp"
#include "render/MapIcons.hpp"
#include "render/PlayerRenderer.hpp"
#include "render/UiFont.hpp"

#include <algorithm>
#include <chrono>
#include <sstream>
#include <string_view>

void GuiWindow::bootstrapState()
{
    _parser.consume(_buffer, _state);
    MapSync::flush(_client, _buffer, _parser, _state,
                   std::chrono::milliseconds(800));
    _animator.reset();
    _animator.update(_state, 0.f);
}

void GuiWindow::pullNetwork()
{
    char chunk[4096];
    while (_client.isConnected())
    {
        const int n = _client.recvRaw(chunk, sizeof(chunk));
        if (n < 0)
            break;
        if (n == 0)
            return;
        _buffer.append(std::string_view(chunk, static_cast<std::size_t>(n)));
    }
    _parser.consume(_buffer, _state);
}

sf::Color GuiWindow::tileColor(int x, int y) const
{
    return ((x + y) % 2 == 0) ? sf::Color(62, 98, 68) : sf::Color(52, 82, 58);
}

sf::Color GuiWindow::teamColor(const std::string &team) const
{
    static const sf::Color colors[] = {
        sf::Color(232, 92, 92),  sf::Color(88, 152, 232),
        sf::Color(232, 188, 72), sf::Color(168, 96, 216),
        sf::Color(88, 210, 168), sf::Color(232, 128, 188),
    };
    std::size_t hash = 0;
    for (char c : team)
        hash = hash * 31 + static_cast<unsigned char>(c);
    return colors[hash % 6];
}

void GuiWindow::drawMap(sf::RenderWindow &window) const
{
    const float tile = _camera.tileSize();
    const float gap = std::max(1.f, tile * 0.04f);
    const float drawSize = tile - gap;

    sf::RectangleShape cell(sf::Vector2f(drawSize, drawSize));
    cell.setOutlineColor(sf::Color(20, 30, 22, 90));
    cell.setOutlineThickness(1.f);

    const int startX = static_cast<int>(_camera.originX());
    const int startY = static_cast<int>(_camera.originY());
    const int endX = startX + _camera.viewTilesX();
    const int endY = startY + _camera.viewTilesY();

    for (int y = startY; y < endY; ++y)
    {
        for (int x = startX; x < endX; ++x)
        {
            cell.setFillColor(tileColor(x, y));
            cell.setPosition(_camera.worldX(x) + gap * 0.5f,
                             _camera.worldY(y) + gap * 0.5f);
            window.draw(cell);
        }
    }
}

void GuiWindow::drawResources(sf::RenderWindow &window) const
{
    const float tile = _camera.tileSize();
    const int startX = static_cast<int>(_camera.originX());
    const int startY = static_cast<int>(_camera.originY());
    const int endX = startX + _camera.viewTilesX() + 1;
    const int endY = startY + _camera.viewTilesY() + 1;

    for (int y = startY; y < endY; ++y)
    {
        if (y < 0 || y >= _state.height)
            continue;
        for (int x = startX; x < endX; ++x)
        {
            if (x < 0 || x >= _state.width)
                continue;
            MapIcons::drawTileResources(window, _state.tileAt(x, y),
                                        _camera.worldX(x), _camera.worldY(y),
                                        tile);
        }
    }
}

void GuiWindow::drawEggs(sf::RenderWindow &window) const
{
    const float tile = _camera.tileSize();
    for (const auto &[id, position] : _state.eggs())
    {
        (void)id;
        if (!_camera.isTileVisible(position.x, position.y))
            continue;
        const float cx = _camera.worldX(position.x) + tile * 0.5f;
        const float cy = _camera.worldY(position.y) + tile * 0.52f;
        MapIcons::drawEgg(window, cx, cy, tile);
    }
}

void GuiWindow::drawPlayers(sf::RenderWindow &window)
{
    const float tile = _camera.tileSize();
    PlayerAnimator::Snapshot snap{};
    for (const auto &[id, player] : _state.players())
    {
        (void)player;
        if (!_animator.snapshot(id, snap))
            continue;

        const int tileX = static_cast<int>(snap.x + 0.5f);
        const int tileY = static_cast<int>(snap.y + 0.5f);
        if (!_camera.isTileVisible(tileX, tileY))
            continue;

        PlayerRenderer::draw(window, snap, teamColor(snap.team), tile);
    }
}

void GuiWindow::updateLayout(sf::RenderWindow &window)
{
    const auto windowSize = window.getSize();
    _mapPixelWidth =
        windowSize.x > Sidebar::kWidth ? windowSize.x - Sidebar::kWidth : 0;
    _mapPixelHeight = windowSize.y;

    _camera.fitToViewport(_mapPixelWidth, _mapPixelHeight, _state.width,
                          _state.height);
    refreshMapView(window);
}

void GuiWindow::refreshMapView(sf::RenderWindow &window)
{
    const auto size = window.getSize();
    _camera.applyToView(_mapView, _mapPixelWidth, _mapPixelHeight, size.x,
                        size.y);
}

void GuiWindow::updatePan(float deltaSeconds)
{
    float dx = 0.f;
    float dy = 0.f;
    if (_panInput.left)
        dx -= kPanSpeedTilesPerSec * deltaSeconds;
    if (_panInput.right)
        dx += kPanSpeedTilesPerSec * deltaSeconds;
    if (_panInput.up)
        dy -= kPanSpeedTilesPerSec * deltaSeconds;
    if (_panInput.down)
        dy += kPanSpeedTilesPerSec * deltaSeconds;
    if (dx != 0.f || dy != 0.f)
        _camera.pan(dx, dy);
}

void GuiWindow::handleClick(const sf::RenderWindow &window, int pixelX,
                            int pixelY)
{
    if (pixelX < 0 || static_cast<unsigned>(pixelX) >= _mapPixelWidth)
        return;

    const sf::Vector2f world =
        window.mapPixelToCoords(sf::Vector2i(pixelX, pixelY), _mapView);
    const int tileX = _camera.tileXFromWorld(world.x);
    const int tileY = _camera.tileYFromWorld(world.y);
    _selection = pickSelection(_state, _animator, tileX, tileY);
    requestSelectionRefresh(_client, _selection);
}

void GuiWindow::handleKeyPress(sf::Keyboard::Key key,
                               sf::Keyboard::Scancode scancode)
{
    if (PanInputState::isMoveKey(key))
        _panInput.setMoveKey(key, true);
    else if (scancode != sf::Keyboard::Scancode::Unknown &&
             PanInputState::isMoveScancode(scancode))
        _panInput.setScancode(scancode, true);
    if (PanInputState::isArrowKey(key))
        _panInput.setArrowKey(key, true);
}

void GuiWindow::handleKeyRelease(sf::Keyboard::Key key,
                                 sf::Keyboard::Scancode scancode)
{
    if (PanInputState::isMoveKey(key))
        _panInput.setMoveKey(key, false);
    if (scancode != sf::Keyboard::Scancode::Unknown &&
        PanInputState::isMoveScancode(scancode))
        _panInput.setScancode(scancode, false);
    if (PanInputState::isArrowKey(key))
        _panInput.setArrowKey(key, false);
}

bool GuiWindow::handleZoomKey(sf::Keyboard::Key key,
                              sf::Keyboard::Scancode scancode)
{
    if (_zoomHandledThisFrame)
        return false;
    if (ZoomInput::isZoomInKey(key, scancode))
    {
        _camera.zoomIn();
        _zoomHandledThisFrame = true;
        return true;
    }
    if (ZoomInput::isZoomOutKey(key, scancode))
    {
        _camera.zoomOut();
        _zoomHandledThisFrame = true;
        return true;
    }
    return false;
}

bool GuiWindow::handleZoomText(char32_t character)
{
    if (_zoomHandledThisFrame)
        return false;
    if (ZoomInput::isZoomInChar(character))
    {
        _camera.zoomIn();
        _zoomHandledThisFrame = true;
        return true;
    }
    if (ZoomInput::isZoomOutChar(character))
    {
        _camera.zoomOut();
        _zoomHandledThisFrame = true;
        return true;
    }
    return false;
}

bool GuiWindow::handleProtocolKey(sf::Keyboard::Key key)
{
    if (_state.isGameOver())
        return false;

    if (key == sf::Keyboard::R)
    {
        GuiRequests::sendMsz(_client);
        GuiRequests::requestMapSync(_client);
        return true;
    }
    if (key == sf::Keyboard::LBracket)
    {
        const int next = std::max(1, _state.timeUnit - 10);
        GuiRequests::sendSst(_client, next);
        return true;
    }
    if (key == sf::Keyboard::RBracket)
    {
        const int next = std::min(10000, _state.timeUnit + 10);
        GuiRequests::sendSst(_client, next);
        return true;
    }
    return false;
}

void GuiWindow::drawSelection(sf::RenderWindow &window) const
{
    if (_selection.kind == Selection::Kind::None)
        return;
    if (!_camera.isTileVisible(_selection.tileX, _selection.tileY))
        return;

    const float tile = _camera.tileSize();
    sf::RectangleShape highlight(sf::Vector2f(tile - 4.f, tile - 4.f));
    highlight.setFillColor(sf::Color(255, 220, 80, 35));
    highlight.setOutlineColor(sf::Color(255, 230, 120, 220));
    highlight.setOutlineThickness(std::max(2.f, tile * 0.05f));
    highlight.setPosition(_camera.worldX(_selection.tileX) + 2.f,
                          _camera.worldY(_selection.tileY) + 2.f);
    window.draw(highlight);
}

void GuiWindow::drawGameOver(sf::RenderWindow &window,
                             unsigned mapPixelWidth) const
{
    if (!_state.isGameOver())
        return;

    const auto size = window.getSize();
    sf::RectangleShape overlay(sf::Vector2f(static_cast<float>(mapPixelWidth),
                                            static_cast<float>(size.y)));
    overlay.setFillColor(sf::Color(0, 0, 0, 170));
    window.draw(overlay);

    sf::RectangleShape banner(
        sf::Vector2f(static_cast<float>(mapPixelWidth) * 0.78f, 96.f));
    banner.setFillColor(sf::Color(28, 32, 40, 240));
    banner.setOutlineColor(sf::Color(255, 255, 255, 200));
    banner.setOutlineThickness(2.f);
    banner.setPosition(static_cast<float>(mapPixelWidth) * 0.11f,
                       static_cast<float>(size.y) * 0.38f);
    window.draw(banner);

    if (UiFont::available())
    {
        const std::string title = "Winner: " + _state.winner();
        sf::Text label(title, UiFont::get(), 26);
        label.setFillColor(sf::Color::White);
        const auto bounds = label.getLocalBounds();
        label.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
        label.setPosition(static_cast<float>(mapPixelWidth) * 0.5f,
                          static_cast<float>(size.y) * 0.38f + 48.f);
        window.draw(label);
    }
}

void GuiWindow::drawPause(sf::RenderWindow &window,
                          unsigned mapPixelWidth) const
{
    if (!_state.isPaused())
        return;

    sf::RectangleShape banner(
        sf::Vector2f(static_cast<float>(mapPixelWidth), 32.f));
    banner.setFillColor(sf::Color(140, 48, 48, 210));
    banner.setPosition(0.f, 0.f);
    window.draw(banner);

    if (!UiFont::available())
        return;

    sf::Text label("GAME PAUSED", UiFont::get(), 17);
    label.setFillColor(sf::Color::White);
    label.setPosition(14.f, 6.f);
    window.draw(label);
}

GuiWindow::GuiWindow(NetworkClient &client, ReceiveBuffer &buffer,
                     const std::string &host, int port)
    : _client(client), _buffer(buffer), _host(host), _port(port)
{
    bootstrapState();
    _animator.update(_state, 0.f);
}

int GuiWindow::run()
{
    if (_state.width <= 0 || _state.height <= 0)
        return 84;

    const unsigned mapContentWidth = static_cast<unsigned>(_state.width) * 40u;
    const unsigned mapContentHeight =
        static_cast<unsigned>(_state.height) * 40u;
    const unsigned windowWidth =
        std::max(kMinWindowWidth, std::max(kDefaultMapWidth, mapContentWidth)) +
        Sidebar::kWidth;
    const unsigned windowHeight = std::max(
        kMinWindowHeight, std::max(kDefaultMapHeight, mapContentHeight));

    std::ostringstream title;
    title << "Zappy GUI - " << _host << ':' << _port << " (" << _state.width
          << 'x' << _state.height << ")";

    sf::RenderWindow window(
        sf::VideoMode(windowWidth, windowHeight), title.str(),
        sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize);
    window.setFramerateLimit(120);
    window.setVerticalSyncEnabled(true);
    window.setKeyRepeatEnabled(false);
    updateLayout(window);

    auto lastFrame = std::chrono::steady_clock::now();

    while (window.isOpen())
    {
        const auto now = std::chrono::steady_clock::now();
        const float delta = std::min(
            0.05f, std::chrono::duration<float>(now - lastFrame).count());
        lastFrame = now;

        if (!_state.isGameOver())
            pullNetwork();
        else
        {
            std::ostringstream winTitle;
            winTitle << "Zappy - winner: " << _state.winner();
            window.setTitle(winTitle.str());
        }

        _animator.update(_state, delta);
        _state.tickEffects(delta);
        updatePan(delta);

        _zoomHandledThisFrame = false;

        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::Resized)
                updateLayout(window);
            if (event.type == sf::Event::TextEntered)
                handleZoomText(event.text.unicode);
            if (event.type == sf::Event::KeyPressed)
            {
                if (event.key.code == sf::Keyboard::Escape)
                    window.close();
                else if (handleProtocolKey(event.key.code))
                    ;
                else if (!handleZoomKey(event.key.code, event.key.scancode))
                    handleKeyPress(event.key.code, event.key.scancode);
            }
            if (event.type == sf::Event::KeyReleased)
                handleKeyRelease(event.key.code, event.key.scancode);
            if (event.type == sf::Event::MouseButtonPressed &&
                event.mouseButton.button == sf::Mouse::Left)
            {
                handleClick(window, event.mouseButton.x, event.mouseButton.y);
            }
        }

        refreshMapView(window);

        window.clear(sf::Color(24, 28, 34));

        window.setView(_mapView);
        drawMap(window);
        drawResources(window);
        drawEggs(window);
        drawPlayers(window);
        _effects.draw(window, _state, _animator, _camera);
        drawSelection(window);
        drawPause(window, _mapPixelWidth);
        drawGameOver(window, _mapPixelWidth);

        window.setView(window.getDefaultView());
        _sidebar.draw(window, _state, _selection, _camera, _mapPixelWidth);
        window.display();
    }

    return 0;
}
