#include "render/ZoomInput.hpp"

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
    if (!expect(ZoomInput::isZoomInChar(U'+'),
                 "typed plus must zoom in on AZERTY"))
        return EXIT_FAILURE;

    if (!expect(ZoomInput::isZoomInChar(U'='),
                 "typed equals must zoom in on AZERTY shift combo"))
        return EXIT_FAILURE;

    if (!expect(ZoomInput::isZoomOutChar(U'-'),
                 "typed minus must zoom out on AZERTY"))
        return EXIT_FAILURE;

    if (!expect(ZoomInput::isZoomInKey(sf::Keyboard::Add,
                                        sf::Keyboard::Scancode::Unknown),
                 "numpad plus must zoom in"))
        return EXIT_FAILURE;

    if (!expect(
            ZoomInput::isZoomOutKey(sf::Keyboard::Unknown,
                                    sf::Keyboard::Scancode::NumpadMinus),
            "numpad minus scancode must zoom out"))
        return EXIT_FAILURE;

    std::cout << "[OK] zoom input tests passed\n";
    return EXIT_SUCCESS;
}
