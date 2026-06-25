#include "render/Sidebar.hpp"
#include "render/UiFont.hpp"

#include <sstream>

namespace
{
const char *kResourceNames[7] = {"food",     "linemate", "deraumere", "sibur",
                                 "mendiane", "phiras",   "thyste"};

const sf::Color kResourceColors[7] = {
    sf::Color(240, 220, 80),  sf::Color(180, 180, 180),
    sf::Color(200, 120, 60),  sf::Color(120, 200, 255),
    sf::Color(120, 220, 120), sf::Color(220, 120, 220),
    sf::Color(160, 120, 255),
};
} // namespace

Sidebar::Sidebar() = default;

void Sidebar::drawLine(sf::RenderWindow &window, const std::string &text,
                       float x, float &y, unsigned char size) const
{
    if (!UiFont::available())
        return;
    sf::Text line(text, UiFont::get(), size);
    line.setFillColor(sf::Color::White);
    line.setPosition(x, y);
    window.draw(line);
    y += static_cast<float>(size) + 4.f;
}

void Sidebar::drawPlayer(sf::RenderWindow &window, const GameState &state,
                         const Selection &selection, float x, float &y) const
{
    const auto it = state.players().find(selection.playerId);
    if (it == state.players().end())
    {
        drawLine(window, "Player not found", x, y);
        return;
    }

    const Player &player = it->second;
    std::ostringstream header;
    header << "Player #" << player.id;
    drawLine(window, header.str(), x, y, 16);
    drawLine(window, "Team: " + player.team, x, y);
    drawLine(window, "Level: " + std::to_string(player.level), x, y);
    drawLine(window,
             "Pos: " + std::to_string(player.x) + ' ' +
                 std::to_string(player.y),
             x, y);
    drawLine(window, "Inventory:", x, y);

    sf::CircleShape dot(4.f);
    for (int resource = 0; resource < 7; ++resource)
    {
        if (player.inventory[resource] <= 0)
            continue;
        dot.setFillColor(kResourceColors[resource]);
        dot.setPosition(x, y + 2.f);
        window.draw(dot);

        std::ostringstream row;
        row << kResourceNames[resource] << ": " << player.inventory[resource];
        drawLine(window, row.str(), x + 14.f, y, 13);
        y += 2.f;
    }
}

void Sidebar::drawTile(sf::RenderWindow &window, const GameState &state,
                       const Selection &selection, float x, float &y) const
{
    std::ostringstream header;
    header << "Tile " << selection.tileX << ' ' << selection.tileY;
    drawLine(window, header.str(), x, y, 16);
    drawLine(window, "Resources:", x, y);

    const Tile &tile = state.tileAt(selection.tileX, selection.tileY);
    sf::CircleShape dot(4.f);
    for (int resource = 0; resource < 7; ++resource)
    {
        if (tile.resources[resource] <= 0)
            continue;
        dot.setFillColor(kResourceColors[resource]);
        dot.setPosition(x, y + 2.f);
        window.draw(dot);

        std::ostringstream row;
        row << kResourceNames[resource] << ": " << tile.resources[resource];
        drawLine(window, row.str(), x + 14.f, y, 13);
        y += 2.f;
    }
}

void Sidebar::draw(sf::RenderWindow &window, const GameState &state,
                   const Selection &selection, unsigned mapPixelWidth) const
{
    const auto winSize = window.getSize();
    sf::RectangleShape panel(sf::Vector2f(static_cast<float>(kWidth),
                                          static_cast<float>(winSize.y)));
    panel.setPosition(static_cast<float>(mapPixelWidth), 0.f);
    panel.setFillColor(sf::Color(28, 32, 38));
    panel.setOutlineColor(sf::Color(70, 80, 95));
    panel.setOutlineThickness(1.f);
    window.draw(panel);

    float y = 12.f;
    const float x = static_cast<float>(mapPixelWidth) + 12.f;

    drawLine(window, "Selection", x, y, 15);
    y += 4.f;

    if (selection.kind == Selection::Kind::None)
    {
        drawLine(window, "Click a tile", x, y);
        drawLine(window, "or player", x, y);
        return;
    }

    if (selection.kind == Selection::Kind::Player)
        drawPlayer(window, state, selection, x, y);
    else
        drawTile(window, state, selection, x, y);
}
