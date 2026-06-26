#include "render/PanInput.hpp"

void PanInputState::recompute()
{
    left = _leftA || _leftQ || _leftScanA || _leftArrow;
    right = _rightD || _rightScanD || _rightArrow;
    up = _upW || _upZ || _upScanW || _upArrow;
    down = _downS || _downScanS || _downArrow;
}

void PanInputState::setMoveKey(sf::Keyboard::Key key, bool pressed)
{
    switch (key)
    {
    case sf::Keyboard::A:
        _leftA = pressed;
        break;
    case sf::Keyboard::Q:
        _leftQ = pressed;
        break;
    case sf::Keyboard::D:
        _rightD = pressed;
        break;
    case sf::Keyboard::W:
        _upW = pressed;
        break;
    case sf::Keyboard::Z:
        _upZ = pressed;
        break;
    case sf::Keyboard::S:
        _downS = pressed;
        break;
    default:
        break;
    }
    recompute();
}

void PanInputState::setScancode(sf::Keyboard::Scancode scancode, bool pressed)
{
    switch (scancode)
    {
    case sf::Keyboard::Scancode::A:
        _leftScanA = pressed;
        break;
    case sf::Keyboard::Scancode::D:
        _rightScanD = pressed;
        break;
    case sf::Keyboard::Scancode::W:
        _upScanW = pressed;
        break;
    case sf::Keyboard::Scancode::S:
        _downScanS = pressed;
        break;
    default:
        break;
    }
    recompute();
}

void PanInputState::setArrowKey(sf::Keyboard::Key key, bool pressed)
{
    switch (key)
    {
    case sf::Keyboard::Left:
        _leftArrow = pressed;
        break;
    case sf::Keyboard::Right:
        _rightArrow = pressed;
        break;
    case sf::Keyboard::Up:
        _upArrow = pressed;
        break;
    case sf::Keyboard::Down:
        _downArrow = pressed;
        break;
    default:
        break;
    }
    recompute();
}

bool PanInputState::isArrowKey(sf::Keyboard::Key key)
{
    return key == sf::Keyboard::Left || key == sf::Keyboard::Right ||
           key == sf::Keyboard::Up || key == sf::Keyboard::Down;
}

bool PanInputState::isMoveKey(sf::Keyboard::Key key)
{
    return key == sf::Keyboard::W || key == sf::Keyboard::Z ||
           key == sf::Keyboard::A || key == sf::Keyboard::Q ||
           key == sf::Keyboard::S || key == sf::Keyboard::D;
}

bool PanInputState::isMoveScancode(sf::Keyboard::Scancode scancode)
{
    return scancode == sf::Keyboard::Scancode::A ||
           scancode == sf::Keyboard::Scancode::D ||
           scancode == sf::Keyboard::Scancode::W ||
           scancode == sf::Keyboard::Scancode::S;
}
