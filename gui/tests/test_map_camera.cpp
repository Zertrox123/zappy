#include "render/MapCamera.hpp"

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
    MapCamera camera;
    camera.configure(20, 15, 10, 8);

    camera.pan(50, 50);
    if (!expect(camera.originX() == 10 && camera.originY() == 7,
                 "camera must clamp pan to map bounds"))
        return EXIT_FAILURE;

    camera.pan(-50, -50);
    if (!expect(camera.originX() == 0 && camera.originY() == 0,
                 "camera must not pan below zero"))
        return EXIT_FAILURE;

    if (!expect(camera.tileXFromScreen(64, 32) == 2,
                 "screen to tile must include camera origin"))
        return EXIT_FAILURE;

    std::cout << "[OK] map camera tests passed\n";
    return EXIT_SUCCESS;
}
