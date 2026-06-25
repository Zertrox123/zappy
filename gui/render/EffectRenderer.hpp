#pragma once

#include "model/GameState.hpp"
#include "render/MapCamera.hpp"
#include "render/PlayerAnimator.hpp"

#include <SFML/Graphics.hpp>

class EffectRenderer
{
  public:
    static constexpr unsigned kTileSize = 32;

    void draw(sf::RenderWindow &window, const GameState &state,
              const PlayerAnimator &animator, const MapCamera &camera) const;

  private:
    void drawExpulsion(sf::RenderWindow &window, const WorldEffect &effect,
                       const MapCamera &camera) const;
    void drawBroadcast(sf::RenderWindow &window, const WorldEffect &effect,
                       const PlayerAnimator &animator,
                       const MapCamera &camera) const;
    void drawIncantation(sf::RenderWindow &window, const WorldEffect &effect,
                         const MapCamera &camera) const;
    void drawFork(sf::RenderWindow &window, const WorldEffect &effect,
                  const MapCamera &camera) const;
    void drawResourceFx(sf::RenderWindow &window, const WorldEffect &effect,
                        const MapCamera &camera, bool drop) const;
    void drawIncantationEnd(sf::RenderWindow &window, const WorldEffect &effect,
                            const MapCamera &camera) const;
    void drawDeath(sf::RenderWindow &window, const WorldEffect &effect,
                   const MapCamera &camera) const;
};
