#pragma once

#include <SFML/Window/Keyboard.hpp>

#include <cstdint>

namespace ZoomInput
{
bool isZoomInChar(char32_t character);
bool isZoomOutChar(char32_t character);
bool isZoomInKey(sf::Keyboard::Key key, sf::Keyboard::Scancode scancode);
bool isZoomOutKey(sf::Keyboard::Key key, sf::Keyboard::Scancode scancode);
} // namespace ZoomInput
