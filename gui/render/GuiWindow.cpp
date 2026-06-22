#include "render/GuiWindow.hpp"

#include <algorithm>
#include <chrono>
#include <sstream>
#include <string_view>
#include <thread>

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

void GuiWindow::drawPlayers(sf::RenderWindow &window) const
{
    sf::CircleShape body(12.f);
    body.setOutlineColor(sf::Color::White);
    body.setOutlineThickness(2.f);

    for (const auto &[id, player] : _state.players())
    {
        (void)id;
        body.setFillColor(teamColor(player.team));
        body.setPosition(static_cast<float>(player.x * kTileSize + 4),
                         static_cast<float>(player.y * kTileSize + 4));
        window.draw(body);
    }
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
          << 'x' << _state.height << ") players=" << _state.players().size();

    sf::RenderWindow window(sf::VideoMode(width, height), title.str(),
                            sf::Style::Titlebar | sf::Style::Close);
    window.setFramerateLimit(60);

    sf::RectangleShape tile(sf::Vector2f(static_cast<float>(kTileSize) - 1.f,
                                         static_cast<float>(kTileSize) - 1.f));

    const int tilesX = static_cast<int>(width / kTileSize);
    const int tilesY = static_cast<int>(height / kTileSize);

    while (window.isOpen())
    {
        pullNetwork();

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

        drawPlayers(window);
        window.display();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    return 0;
}
