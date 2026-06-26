#include "render/ZoomInput.hpp"

namespace ZoomInput
{
bool isZoomInChar(char32_t character)
{
    return character == U'+' || character == U'=';
}

bool isZoomOutChar(char32_t character) { return character == U'-'; }

bool isZoomInKey(sf::Keyboard::Key key, sf::Keyboard::Scancode scancode)
{
    if (key == sf::Keyboard::Add)
        return true;
    if (scancode == sf::Keyboard::Scancode::NumpadPlus)
        return true;
    if (scancode == sf::Keyboard::Scancode::Equal)
        return true;
    return false;
}

bool isZoomOutKey(sf::Keyboard::Key key, sf::Keyboard::Scancode scancode)
{
    if (key == sf::Keyboard::Subtract)
        return true;
    if (scancode == sf::Keyboard::Scancode::NumpadMinus)
        return true;
    if (scancode == sf::Keyboard::Scancode::Hyphen)
        return true;
    return false;
}
} // namespace ZoomInput
