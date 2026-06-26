#pragma once

#include "model/Tile.hpp"

#include <SFML/Graphics.hpp>

namespace MapIcons
{
struct ResourceStyle
{
    sf::Color fill;
    sf::Color highlight;
    sf::Color outline;
};

ResourceStyle resourceStyle(int resourceType);

void drawResource(sf::RenderWindow &window, int resourceType, float centerX,
                  float centerY, float tileSize, int quantity = 1);

void drawTileResources(sf::RenderWindow &window, const Tile &tile,
                       float originX, float originY, float tileSize);

void drawEgg(sf::RenderWindow &window, float centerX, float centerY,
             float tileSize);

sf::Vector2f resourceSlot(int resourceType, float tileSize, float originX,
                          float originY);

} // namespace MapIcons
