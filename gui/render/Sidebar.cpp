#include "render/Sidebar.hpp"
#include "render/UiFont.hpp"

#include <algorithm>
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

void Sidebar::drawHud(sf::RenderWindow &window, const GameState &state, float x,
                      float &y) const
{
    drawLine(window, "HUD", x, y, 15);
    drawLine(window, "Freq: " + std::to_string(state.timeUnit), x, y);
    drawLine(window,
             "Players: " + std::to_string(state.players().size()) +
                 "  Eggs: " + std::to_string(state.eggs().size()),
             x, y);
    drawLine(window, "Teams: " + std::to_string(state.teams().size()), x, y);
    drawLine(window,
             "Tiles: " + std::to_string(state.knownTileCount()) + '/' +
                 std::to_string(std::max(0, state.width * state.height)),
             x, y);
    y += 6.f;

    if (!state.serverMessages().empty())
    {
        drawLine(window, "Server:", x, y);
        for (const std::string &message : state.serverMessages())
            drawLine(window, "- " + message, x, y, 12);
        y += 4.f;
    }
}

void Sidebar::drawMinimap(sf::RenderWindow &window, const GameState &state,
                          const MapCamera &camera, float x, float y,
                          unsigned mapPixelWidth) const
{
    (void)mapPixelWidth;
    if (state.width <= 0 || state.height <= 0)
        return;

    const float boxW = static_cast<float>(kWidth) - 24.f;
    const float boxH = 90.f;
    const float scale = std::min(boxW / static_cast<float>(state.width),
                                 boxH / static_cast<float>(state.height));

    sf::RectangleShape frame(sf::Vector2f(boxW, boxH));
    frame.setPosition(x, y);
    frame.setFillColor(sf::Color(18, 22, 28));
    frame.setOutlineColor(sf::Color(90, 100, 120));
    frame.setOutlineThickness(1.f);
    window.draw(frame);

    sf::RectangleShape dot(
        sf::Vector2f(std::max(1.f, scale), std::max(1.f, scale)));
    for (const auto &[id, player] : state.players())
    {
        (void)id;
        dot.setFillColor(sf::Color(220, 80, 80));
        dot.setPosition(x + player.x * scale, y + player.y * scale);
        window.draw(dot);
    }

    sf::RectangleShape view(
        sf::Vector2f(camera.viewTilesX() * scale, camera.viewTilesY() * scale));
    view.setPosition(x + camera.originX() * scale,
                     y + camera.originY() * scale);
    view.setFillColor(sf::Color::Transparent);
    view.setOutlineColor(sf::Color(255, 220, 80));
    view.setOutlineThickness(1.f);
    window.draw(view);

    float labelY = y + boxH + 6.f;
    drawLine(window, "Arrows pan map", x, labelY, 12);
}

void Sidebar::draw(sf::RenderWindow &window, const GameState &state,
                   const Selection &selection, const MapCamera &camera,
                   unsigned mapPixelWidth) const
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

    drawHud(window, state, x, y);
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

    drawMinimap(window, state, camera, x, y + 8.f, mapPixelWidth);
}
