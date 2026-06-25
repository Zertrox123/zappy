#pragma once

#include "model/GameState.hpp"
#include "render/MapCamera.hpp"
#include "render/Selection.hpp"

#include <SFML/Graphics.hpp>

class Sidebar
{
  public:
    static constexpr unsigned kWidth = 200;

    Sidebar();

    void draw(sf::RenderWindow &window, const GameState &state,
              const Selection &selection, const MapCamera &camera,
              unsigned mapPixelWidth) const;

  private:
    void drawHud(sf::RenderWindow &window, const GameState &state, float x,
                 float &y) const;
    void drawMinimap(sf::RenderWindow &window, const GameState &state,
                     const MapCamera &camera, float x, float y,
                     unsigned mapPixelWidth) const;
    void drawLine(sf::RenderWindow &window, const std::string &text, float x,
                  float &y, unsigned char size = 14) const;
    void drawPlayer(sf::RenderWindow &window, const GameState &state,
                    const Selection &selection, float x, float &y) const;
    void drawTile(sf::RenderWindow &window, const GameState &state,
                  const Selection &selection, float x, float &y) const;
};
