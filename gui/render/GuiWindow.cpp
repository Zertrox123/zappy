#include "render/GuiWindow.hpp"
#include "protocol/MapSync.hpp"
#include "render/PlayerRenderer.hpp"
#include "render/UiFont.hpp"

#include <algorithm>
#include <chrono>
#include <sstream>
#include <string_view>
#include <thread>

namespace
{
const sf::Color kResourceColors[7] = {
    sf::Color(240, 220, 80),  sf::Color(180, 180, 180),
    sf::Color(200, 120, 60),  sf::Color(120, 200, 255),
    sf::Color(120, 220, 120), sf::Color(220, 120, 220),
    sf::Color(160, 120, 255),
};

const sf::Vector2f kResourceOffsets[7] = {
    {8.f, 8.f},  {20.f, 6.f}, {24.f, 16.f}, {20.f, 24.f},
    {8.f, 24.f}, {4.f, 16.f}, {16.f, 16.f},
};

bool inView(int tile, int origin, int viewTiles)
{
    return tile >= origin && tile < origin + viewTiles;
}
} // namespace

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
    return ((x + y) % 2 == 0) ? sf::Color(56, 86, 56) : sf::Color(46, 72, 46);
}

sf::Color GuiWindow::teamColor(const std::string &team) const
{
    static const sf::Color colors[] = {
        sf::Color(220, 80, 80),  sf::Color(80, 140, 220),
        sf::Color(220, 180, 60), sf::Color(160, 80, 200),
        sf::Color(80, 200, 160), sf::Color(220, 120, 180),
    };
    std::size_t hash = 0;
    for (char c : team)
        hash = hash * 31 + static_cast<unsigned char>(c);
    return colors[hash % 6];
}

void GuiWindow::drawMap(sf::RenderWindow &window,
                        sf::RectangleShape &tile) const
{
    for (int y = _camera.originY();
         y < _camera.originY() + _camera.viewTilesY(); ++y)
    {
        for (int x = _camera.originX();
             x < _camera.originX() + _camera.viewTilesX(); ++x)
        {
            tile.setFillColor(tileColor(x, y));
            tile.setPosition(_camera.screenX(x, kTileSize),
                             _camera.screenY(y, kTileSize));
            window.draw(tile);
        }
    }
}

void GuiWindow::drawResources(sf::RenderWindow &window) const
{
    sf::ConvexShape gem(4);
    gem.setPoint(0, {0.f, -4.f});
    gem.setPoint(1, {4.f, 0.f});
    gem.setPoint(2, {0.f, 4.f});
    gem.setPoint(3, {-4.f, 0.f});

    for (int y = _camera.originY();
         y < _camera.originY() + _camera.viewTilesY(); ++y)
    {
        if (y < 0 || y >= _state.height)
            continue;
        for (int x = _camera.originX();
             x < _camera.originX() + _camera.viewTilesX(); ++x)
        {
            if (x < 0 || x >= _state.width)
                continue;
            const Tile &tile = _state.tileAt(x, y);
            const float baseX = _camera.screenX(x, kTileSize);
            const float baseY = _camera.screenY(y, kTileSize);

            for (int resource = 0; resource < 7; ++resource)
            {
                if (tile.resources[resource] <= 0)
                    continue;
                gem.setFillColor(kResourceColors[resource]);
                gem.setPosition(baseX + kResourceOffsets[resource].x,
                                baseY + kResourceOffsets[resource].y);
                window.draw(gem);
            }
        }
    }
}

void GuiWindow::drawEggs(sf::RenderWindow &window) const
{
    sf::CircleShape egg(6.f);
    egg.setFillColor(sf::Color(240, 230, 120));
    egg.setOutlineColor(sf::Color::White);
    egg.setOutlineThickness(1.f);

    for (const auto &[id, position] : _state.eggs())
    {
        (void)id;
        if (!inView(position.x, _camera.originX(), _camera.viewTilesX()) ||
            !inView(position.y, _camera.originY(), _camera.viewTilesY()))
            continue;
        egg.setPosition(_camera.screenX(position.x, kTileSize) + 10.f,
                        _camera.screenY(position.y, kTileSize) + 10.f);
        window.draw(egg);
    }
}

void GuiWindow::drawPlayers(sf::RenderWindow &window)
{
    PlayerAnimator::Snapshot snap{};
    for (const auto &[id, player] : _state.players())
    {
        (void)player;
        if (!_animator.snapshot(id, snap))
            continue;

        const int tileX = static_cast<int>(snap.x + 0.5f);
        const int tileY = static_cast<int>(snap.y + 0.5f);
        if (!inView(tileX, _camera.originX(), _camera.viewTilesX()) ||
            !inView(tileY, _camera.originY(), _camera.viewTilesY()))
            continue;

        PlayerAnimator::Snapshot viewSnap = snap;
        viewSnap.x =
            _camera.screenX(tileX, kTileSize) / static_cast<float>(kTileSize);
        viewSnap.y =
            _camera.screenY(tileY, kTileSize) / static_cast<float>(kTileSize);
        PlayerRenderer::draw(window, viewSnap, teamColor(snap.team));
    }
}

void GuiWindow::handleClick(int pixelX, int pixelY, unsigned mapPixelWidth)
{
    if (pixelX < 0 || static_cast<unsigned>(pixelX) >= mapPixelWidth)
        return;

    const int tileX = _camera.tileXFromScreen(pixelX, kTileSize);
    const int tileY = _camera.tileYFromScreen(pixelY, kTileSize);
    _selection = pickSelection(_state, _animator, tileX, tileY);
    requestSelectionRefresh(_client, _selection);
}

void GuiWindow::handleKey(sf::Keyboard::Key key)
{
    switch (key)
    {
    case sf::Keyboard::Left:
        _camera.pan(-1, 0);
        break;
    case sf::Keyboard::Right:
        _camera.pan(1, 0);
        break;
    case sf::Keyboard::Up:
        _camera.pan(0, -1);
        break;
    case sf::Keyboard::Down:
        _camera.pan(0, 1);
        break;
    default:
        break;
    }
}

void GuiWindow::drawSelection(sf::RenderWindow &window) const
{
    if (_selection.kind == Selection::Kind::None)
        return;
    if (!inView(_selection.tileX, _camera.originX(), _camera.viewTilesX()) ||
        !inView(_selection.tileY, _camera.originY(), _camera.viewTilesY()))
        return;

    sf::RectangleShape highlight(
        sf::Vector2f(static_cast<float>(kTileSize) - 1.f,
                     static_cast<float>(kTileSize) - 1.f));
    highlight.setFillColor(sf::Color::Transparent);
    highlight.setOutlineColor(sf::Color(255, 220, 80));
    highlight.setOutlineThickness(2.f);
    highlight.setPosition(_camera.screenX(_selection.tileX, kTileSize),
                          _camera.screenY(_selection.tileY, kTileSize));
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
    overlay.setFillColor(sf::Color(0, 0, 0, 160));
    window.draw(overlay);

    sf::RectangleShape banner(
        sf::Vector2f(static_cast<float>(mapPixelWidth) * 0.8f, 80.f));
    banner.setFillColor(sf::Color(40, 40, 40, 230));
    banner.setOutlineColor(sf::Color::White);
    banner.setOutlineThickness(2.f);
    banner.setPosition(static_cast<float>(mapPixelWidth) * 0.1f,
                       static_cast<float>(size.y) * 0.4f);
    window.draw(banner);

    if (UiFont::available())
    {
        const std::string title = "Winner: " + _state.winner();
        sf::Text label(title, UiFont::get(), 22);
        label.setFillColor(sf::Color::White);
        const auto bounds = label.getLocalBounds();
        label.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
        label.setPosition(static_cast<float>(mapPixelWidth) * 0.5f,
                          static_cast<float>(size.y) * 0.4f + 40.f);
        window.draw(label);
    }
}

void GuiWindow::drawPause(sf::RenderWindow &window,
                          unsigned mapPixelWidth) const
{
    if (!_state.isPaused())
        return;

    sf::RectangleShape banner(
        sf::Vector2f(static_cast<float>(mapPixelWidth), 28.f));
    banner.setFillColor(sf::Color(120, 40, 40, 200));
    banner.setPosition(0.f, 0.f);
    window.draw(banner);

    if (!UiFont::available())
        return;

    sf::Text label("GAME PAUSED", UiFont::get(), 16);
    label.setFillColor(sf::Color::White);
    label.setPosition(12.f, 4.f);
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

    const int viewTilesX =
        std::min(_state.width, static_cast<int>(kMaxWindow / kTileSize));
    const int viewTilesY =
        std::min(_state.height, static_cast<int>(kMaxWindow / kTileSize));
    _camera.configure(_state.width, _state.height, viewTilesX, viewTilesY);

    const unsigned mapPixelWidth =
        static_cast<unsigned>(viewTilesX * kTileSize);
    const unsigned mapPixelHeight =
        static_cast<unsigned>(viewTilesY * kTileSize);
    const unsigned windowWidth = mapPixelWidth + Sidebar::kWidth;
    const unsigned windowHeight = mapPixelHeight;

    std::ostringstream title;
    title << "Zappy GUI - " << _host << ':' << _port << " (" << _state.width
          << 'x' << _state.height << ")";

    sf::RenderWindow window(sf::VideoMode(windowWidth, windowHeight),
                            title.str(),
                            sf::Style::Titlebar | sf::Style::Close);
    window.setFramerateLimit(60);

    sf::RectangleShape tile(sf::Vector2f(static_cast<float>(kTileSize) - 1.f,
                                         static_cast<float>(kTileSize) - 1.f));
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

        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::KeyPressed)
            {
                if (event.key.code == sf::Keyboard::Escape)
                    window.close();
                else
                    handleKey(event.key.code);
            }
            if (event.type == sf::Event::MouseButtonPressed &&
                event.mouseButton.button == sf::Mouse::Left)
            {
                handleClick(event.mouseButton.x, event.mouseButton.y,
                            mapPixelWidth);
            }
        }

        window.clear(sf::Color(30, 30, 30));
        drawMap(window, tile);
        drawResources(window);
        drawEggs(window);
        drawPlayers(window);
        _effects.draw(window, _state, _animator, _camera);
        drawSelection(window);
        drawPause(window, mapPixelWidth);
        drawGameOver(window, mapPixelWidth);
        _sidebar.draw(window, _state, _selection, _camera, mapPixelWidth);
        window.display();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    return 0;
}
