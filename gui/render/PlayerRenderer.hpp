#pragma once

#include "render/PlayerAnimator.hpp"

#include <SFML/Graphics.hpp>

class PlayerRenderer
{
  public:
    static void draw(sf::RenderWindow &window,
                     const PlayerAnimator::Snapshot &snap,
                     const sf::Color &color, float tileSize);
};
