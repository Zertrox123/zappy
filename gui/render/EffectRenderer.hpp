#pragma once

#include "model/GameState.hpp"
#include "render/PlayerAnimator.hpp"

#include <SFML/Graphics.hpp>

class EffectRenderer
{
  public:
    static constexpr unsigned kTileSize = 32;

    void draw(sf::RenderWindow &window, const GameState &state,
              const PlayerAnimator &animator) const;

  private:
    void drawExpulsion(sf::RenderWindow &window,
                       const WorldEffect &effect) const;
    void drawBroadcast(sf::RenderWindow &window, const WorldEffect &effect,
                       const PlayerAnimator &animator) const;
    void drawIncantation(sf::RenderWindow &window,
                         const WorldEffect &effect) const;
};
