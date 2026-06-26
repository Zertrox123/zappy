#include "render/PanInput.hpp"

#include <cstdlib>
#include <iostream>

namespace
{
bool expect(bool condition, const char *message)
{
    if (!condition)
    {
        std::cerr << "[FAIL] " << message << '\n';
        return false;
    }
    return true;
}
} // namespace

int main()
{
    PanInputState input;
    input.setMoveKey(sf::Keyboard::W, true);
    if (!expect(input.up, "W must pan up on QWERTY"))
        return EXIT_FAILURE;

    input.setMoveKey(sf::Keyboard::W, false);
    input.setMoveKey(sf::Keyboard::Z, true);
    if (!expect(input.up, "Z must pan up on AZERTY"))
        return EXIT_FAILURE;

    input.setMoveKey(sf::Keyboard::Q, true);
    if (!expect(input.left, "Q must pan left on AZERTY"))
        return EXIT_FAILURE;

    input.setScancode(sf::Keyboard::Scancode::D, true);
    if (!expect(input.right, "physical right key must pan right"))
        return EXIT_FAILURE;

    input.setArrowKey(sf::Keyboard::Up, true);
    if (!expect(input.up, "arrow keys must still pan"))
        return EXIT_FAILURE;

    input.setMoveKey(sf::Keyboard::Z, true);
    input.setMoveKey(sf::Keyboard::Z, false);
    if (!expect(input.up, "up stays active while arrow held"))
        return EXIT_FAILURE;

    input.setArrowKey(sf::Keyboard::Up, false);
    if (!expect(!input.up, "release must clear direction when no key held"))
        return EXIT_FAILURE;

    std::cout << "[OK] pan input tests passed\n";
    return EXIT_SUCCESS;
}
