#include "render/MapCamera.hpp"

#include <cmath>
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
    camera.fitToViewport(640, 600, 20, 15);

    camera.pan(50.f, 50.f);
    const float maxX =
        std::max(0.f, 20.f - static_cast<float>(camera.viewTilesX()));
    const float maxY =
        std::max(0.f, 15.f - static_cast<float>(camera.viewTilesY()));
    if (!expect(std::abs(camera.originX() - maxX) < 0.01f &&
                     std::abs(camera.originY() - maxY) < 0.01f,
                 "camera must clamp pan to map bounds"))
        return EXIT_FAILURE;

    camera.pan(-50.f, -50.f);
    if (!expect(camera.originX() < 0.01f && camera.originY() < 0.01f,
                 "camera must not pan below zero"))
        return EXIT_FAILURE;

    if (!expect(camera.tileXFromWorld(camera.worldX(2) + camera.tileSize() * 0.5f) == 2,
                 "world to tile must round correctly"))
        return EXIT_FAILURE;

    sf::View view;
    camera.applyToView(view, 640, 600, 900, 600);
    const auto viewport = view.getViewport();
    const float viewportAspect =
        (viewport.width * 900.f) / (viewport.height * 600.f);
    const auto viewSize = view.getSize();
    const float viewAspect = viewSize.x / viewSize.y;
    if (!expect(std::abs(viewportAspect - viewAspect) < 0.02f,
                 "viewport aspect must match view to avoid stretch"))
        return EXIT_FAILURE;

    if (!expect(std::abs(viewport.top) < 0.01f && viewport.height > 0.99f,
                 "map view must fill the window height"))
        return EXIT_FAILURE;

    if (!expect(std::abs(viewSize.x - 640.f) < 0.5f &&
                     std::abs(viewSize.y - 600.f) < 0.5f,
                 "view must exactly cover the map pixel area"))
        return EXIT_FAILURE;

    const float initialTileSize = camera.tileSize();
    const int initialTilesX = camera.viewTilesX();

    MapCamera zoomCamera;
    zoomCamera.fitToViewport(1280, 800, 50, 50);
    const float zoomBaseTileSize = zoomCamera.tileSize();
    const int zoomBaseTilesX = zoomCamera.viewTilesX();

    zoomCamera.zoomOut();
    const float zoomedOutTileSize = zoomCamera.tileSize();
    const int zoomedOutTilesX = zoomCamera.viewTilesX();
    if (!expect(zoomedOutTileSize < zoomBaseTileSize &&
                     zoomedOutTilesX > zoomBaseTilesX,
                 "zoom out must shrink tiles and show more tiles"))
        return EXIT_FAILURE;

    zoomCamera.zoomIn();
    if (!expect(zoomCamera.tileSize() > zoomedOutTileSize &&
                     zoomCamera.viewTilesX() < zoomedOutTilesX,
                 "zoom in must enlarge tiles and show fewer tiles"))
        return EXIT_FAILURE;

    (void)initialTileSize;
    (void)initialTilesX;

    std::cout << "[OK] map camera tests passed\n";
    return EXIT_SUCCESS;
}
