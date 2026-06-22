#include "render/GuiWindow.hpp"

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
} // namespace

void GuiWindow::bootstrapState() { _parser.consume(_buffer, _state); }

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

void GuiWindow::drawResources(sf::RenderWindow &window) const
{
    sf::CircleShape dot(3.f);
    const int tilesY =
        std::min(_state.height, static_cast<int>(_state.tiles().size()));

    for (int y = 0; y < tilesY; ++y)
    {
        const int tilesX =
            std::min(_state.width, static_cast<int>(_state.tiles()[y].size()));
        for (int x = 0; x < tilesX; ++x)
        {
            const Tile &tile = _state.tiles()[static_cast<std::size_t>(y)]
                                             [static_cast<std::size_t>(x)];
            const float baseX = static_cast<float>(x * kTileSize);
            const float baseY = static_cast<float>(y * kTileSize);

            for (int resource = 0; resource < 7; ++resource)
            {
                if (tile.resources[resource] <= 0)
                    continue;
                dot.setFillColor(kResourceColors[resource]);
                dot.setPosition(baseX + kResourceOffsets[resource].x,
                                baseY + kResourceOffsets[resource].y);
                window.draw(dot);
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
        egg.setPosition(static_cast<float>(position.x * kTileSize + 10),
                        static_cast<float>(position.y * kTileSize + 10));
        window.draw(egg);
    }
}

void GuiWindow::drawPlayers(sf::RenderWindow &window) const
{
    sf::CircleShape body(12.f);
    body.setOutlineColor(sf::Color::White);
    body.setOutlineThickness(2.f);

    sf::CircleShape facing(4.f);
    facing.setFillColor(sf::Color::White);

    for (const auto &[id, player] : _state.players())
    {
        (void)id;
        const float cx = static_cast<float>(player.x * kTileSize + 16);
        const float cy = static_cast<float>(player.y * kTileSize + 16);
        body.setFillColor(teamColor(player.team));
        body.setPosition(cx - 12.f, cy - 12.f);
        window.draw(body);

        float dx = 0.f;
        float dy = 0.f;
        switch (player.orientation)
        {
        case 1:
            dy = -10.f;
            break;
        case 2:
            dx = 10.f;
            break;
        case 3:
            dy = 10.f;
            break;
        case 4:
            dx = -10.f;
            break;
        default:
            break;
        }
        facing.setPosition(cx + dx - 4.f, cy + dy - 4.f);
        window.draw(facing);
    }
}

void GuiWindow::drawGameOver(sf::RenderWindow &window) const
{
    if (!_state.isGameOver())
        return;

    const auto size = window.getSize();
    sf::RectangleShape overlay(
        sf::Vector2f(static_cast<float>(size.x), static_cast<float>(size.y)));
    overlay.setFillColor(sf::Color(0, 0, 0, 160));
    window.draw(overlay);

    sf::RectangleShape banner(
        sf::Vector2f(static_cast<float>(size.x) * 0.8f, 80.f));
    banner.setFillColor(sf::Color(40, 40, 40, 230));
    banner.setOutlineColor(sf::Color::White);
    banner.setOutlineThickness(2.f);
    banner.setPosition(static_cast<float>(size.x) * 0.1f,
                       static_cast<float>(size.y) * 0.4f);
    window.draw(banner);
}

GuiWindow::GuiWindow(NetworkClient &client, ReceiveBuffer &buffer,
                     const std::string &host, int port)
    : _client(client), _buffer(buffer), _host(host), _port(port)
{
    bootstrapState();
}

int GuiWindow::run()
{
    if (_state.width <= 0 || _state.height <= 0)
        return 84;

    const unsigned width =
        std::min(static_cast<unsigned>(_state.width) * kTileSize, kMaxWindow);
    const unsigned height =
        std::min(static_cast<unsigned>(_state.height) * kTileSize, kMaxWindow);

    std::ostringstream title;
    title << "Zappy GUI - " << _host << ':' << _port << " (" << _state.width
          << 'x' << _state.height << ")";

    sf::RenderWindow window(sf::VideoMode(width, height), title.str(),
                            sf::Style::Titlebar | sf::Style::Close);
    window.setFramerateLimit(60);

    sf::RectangleShape tile(sf::Vector2f(static_cast<float>(kTileSize) - 1.f,
                                         static_cast<float>(kTileSize) - 1.f));

    const int tilesX = static_cast<int>(width / kTileSize);
    const int tilesY = static_cast<int>(height / kTileSize);

    while (window.isOpen())
    {
        if (!_state.isGameOver())
            pullNetwork();
        else
        {
            std::ostringstream winTitle;
            winTitle << "Zappy - winner: " << _state.winner();
            window.setTitle(winTitle.str());
        }

        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::KeyPressed &&
                event.key.code == sf::Keyboard::Escape)
                window.close();
        }

        window.clear(sf::Color(30, 30, 30));

        for (int y = 0; y < tilesY; ++y)
        {
            for (int x = 0; x < tilesX; ++x)
            {
                tile.setFillColor(tileColor(x, y));
                tile.setPosition(static_cast<float>(x * kTileSize),
                                 static_cast<float>(y * kTileSize));
                window.draw(tile);
            }
        }

        drawResources(window);
        drawEggs(window);
        drawPlayers(window);
        drawGameOver(window);
        window.display();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    return 0;
}
