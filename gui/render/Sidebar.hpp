#pragma once

#include "model/GameState.hpp"
#include "render/Selection.hpp"

#include <SFML/Graphics.hpp>

class Sidebar
{
  public:
    static constexpr unsigned kWidth = 200;

    Sidebar();

    void draw(sf::RenderWindow &window, const GameState &state,
              const Selection &selection, unsigned mapPixelWidth) const;

  private:
    void drawLine(sf::RenderWindow &window, const std::string &text, float x,
                  float &y, unsigned char size = 14) const;
    void drawPlayer(sf::RenderWindow &window, const GameState &state,
                    const Selection &selection, float x, float &y) const;
    void drawTile(sf::RenderWindow &window, const GameState &state,
                  const Selection &selection, float x, float &y) const;
};
