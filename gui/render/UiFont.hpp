#pragma once

#include <SFML/Graphics.hpp>

class UiFont
{
  public:
    static bool available();
    static const sf::Font &get();

  private:
    UiFont();
    static UiFont &instance();

    sf::Font _font;
    bool _loaded = false;
};
