#pragma once

#include <SFML/Window/Keyboard.hpp>

struct PanInputState
{
    bool left = false;
    bool right = false;
    bool up = false;
    bool down = false;

    void setMoveKey(sf::Keyboard::Key key, bool pressed);
    void setScancode(sf::Keyboard::Scancode scancode, bool pressed);
    void setArrowKey(sf::Keyboard::Key key, bool pressed);
    void recompute();

    static bool isArrowKey(sf::Keyboard::Key key);
    static bool isMoveKey(sf::Keyboard::Key key);
    static bool isMoveScancode(sf::Keyboard::Scancode scancode);

  private:
    bool _leftA = false;
    bool _leftQ = false;
    bool _leftScanA = false;
    bool _leftArrow = false;

    bool _rightD = false;
    bool _rightScanD = false;
    bool _rightArrow = false;

    bool _upW = false;
    bool _upZ = false;
    bool _upScanW = false;
    bool _upArrow = false;

    bool _downS = false;
    bool _downScanS = false;
    bool _downArrow = false;
};
