#include "render/MapCamera.hpp"

#include <algorithm>

void MapCamera::configure(int mapWidth, int mapHeight, int viewTilesX,
                          int viewTilesY)
{
    _mapWidth = mapWidth;
    _mapHeight = mapHeight;
    _viewTilesX = viewTilesX;
    _viewTilesY = viewTilesY;
    _originX = std::clamp(_originX, 0, std::max(0, mapWidth - viewTilesX));
    _originY = std::clamp(_originY, 0, std::max(0, mapHeight - viewTilesY));
}

void MapCamera::pan(int deltaTilesX, int deltaTilesY)
{
    _originX = std::clamp(_originX + deltaTilesX, 0,
                          std::max(0, _mapWidth - _viewTilesX));
    _originY = std::clamp(_originY + deltaTilesY, 0,
                          std::max(0, _mapHeight - _viewTilesY));
}

float MapCamera::screenX(int tileX, unsigned tileSize) const
{
    return static_cast<float>((tileX - _originX) * static_cast<int>(tileSize));
}

float MapCamera::screenY(int tileY, unsigned tileSize) const
{
    return static_cast<float>((tileY - _originY) * static_cast<int>(tileSize));
}

int MapCamera::tileXFromScreen(int pixelX, unsigned tileSize) const
{
    return _originX + pixelX / static_cast<int>(tileSize);
}

int MapCamera::tileYFromScreen(int pixelY, unsigned tileSize) const
{
    return _originY + pixelY / static_cast<int>(tileSize);
}
