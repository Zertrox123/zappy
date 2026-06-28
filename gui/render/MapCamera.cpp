#include "render/MapCamera.hpp"

#include <algorithm>
#include <cmath>

namespace
{
unsigned greatestCommonDivisor(unsigned a, unsigned b)
{
    while (b != 0)
    {
        const unsigned remainder = a % b;
        a = b;
        b = remainder;
    }
    return a;
}

int viewTilesWithPanRoom(int mapSize, int wantedTiles)
{
    if (mapSize <= wantedTiles)
    {
        if (mapSize > 6)
            return std::max(6, mapSize - 2);
        return mapSize;
    }
    return wantedTiles;
}
} // namespace

void MapCamera::fitToViewport(unsigned pixelWidth, unsigned pixelHeight,
                              int mapWidth, int mapHeight)
{
    _mapWidth = mapWidth;
    _mapHeight = mapHeight;
    _pixelWidth = pixelWidth;
    _pixelHeight = pixelHeight;

    if (_pixelWidth == 0 || _pixelHeight == 0)
        return;

    const unsigned divisor = greatestCommonDivisor(_pixelWidth, _pixelHeight);
    _aspectNumerator = static_cast<int>(_pixelWidth / divisor);
    _aspectDenominator = static_cast<int>(_pixelHeight / divisor);

    const int previousScale = _scale;
    updateScaleLimits();

    if (previousScale == 0)
    {
        _scale = std::clamp(
            static_cast<int>(std::floor(
                static_cast<float>(_pixelWidth) /
                (kTargetTileSize * static_cast<float>(_aspectNumerator)))),
            _minScale, _maxScale);
        if (_scale == _maxScale && _maxScale > _minScale)
            _scale = std::max(_minScale, _maxScale - 1);
    }
    else
        _scale = std::clamp(previousScale, _minScale, _maxScale);

    rebuildView();
    clampOrigin();
}

void MapCamera::updateScaleLimits()
{
    const int wantedX =
        std::max(1, static_cast<int>(_pixelWidth / kTargetTileSize));
    const int wantedY =
        std::max(1, static_cast<int>(_pixelHeight / kTargetTileSize));
    const int maxTilesX = viewTilesWithPanRoom(_mapWidth, wantedX);
    const int maxTilesY = viewTilesWithPanRoom(_mapHeight, wantedY);

    _maxScale = std::min(std::max(1, maxTilesX / _aspectNumerator),
                         std::max(1, maxTilesY / _aspectDenominator));
    _maxScale = std::min(
        _maxScale,
        std::max(1,
                 static_cast<int>(std::floor(
                     static_cast<float>(_pixelWidth) /
                     (kMinTileSize * static_cast<float>(_aspectNumerator))))));

    _minScale = std::max(
        1, static_cast<int>(std::ceil(
               static_cast<float>(_pixelWidth) /
               (kMaxTileSize * static_cast<float>(_aspectNumerator)))));
    _minScale = std::min(_minScale, _maxScale);
}

void MapCamera::rebuildView()
{
    _viewTilesX = _scale * _aspectNumerator;
    _viewTilesY = _scale * _aspectDenominator;
    _tileSize =
        static_cast<float>(_pixelWidth) / static_cast<float>(_viewTilesX);
}

void MapCamera::zoomIn()
{
    if (_scale <= _minScale)
        return;
    --_scale;
    rebuildView();
    clampOrigin();
}

void MapCamera::zoomOut()
{
    if (_scale >= _maxScale)
        return;
    ++_scale;
    rebuildView();
    clampOrigin();
}

void MapCamera::clampOrigin()
{
    const float maxX =
        std::max(0.f, static_cast<float>(_mapWidth - _viewTilesX));
    const float maxY =
        std::max(0.f, static_cast<float>(_mapHeight - _viewTilesY));
    _originX = std::clamp(_originX, 0.f, maxX);
    _originY = std::clamp(_originY, 0.f, maxY);
}

void MapCamera::pan(float deltaTilesX, float deltaTilesY)
{
    _originX += deltaTilesX;
    _originY += deltaTilesY;
    clampOrigin();
}

void MapCamera::applyToView(sf::View &view, unsigned mapPixelWidth,
                            unsigned mapPixelHeight, unsigned windowWidth,
                            unsigned windowHeight) const
{
    const float contentWidth = static_cast<float>(_viewTilesX) * _tileSize;
    const float contentHeight = static_cast<float>(_viewTilesY) * _tileSize;
    view.reset(sf::FloatRect(_originX * _tileSize, _originY * _tileSize,
                             contentWidth, contentHeight));
    view.setViewport(sf::FloatRect(
        0.f, 0.f,
        static_cast<float>(mapPixelWidth) / static_cast<float>(windowWidth),
        static_cast<float>(mapPixelHeight) / static_cast<float>(windowHeight)));
}

float MapCamera::worldX(int tileX) const
{
    return static_cast<float>(tileX) * _tileSize;
}

float MapCamera::worldY(int tileY) const
{
    return static_cast<float>(tileY) * _tileSize;
}

float MapCamera::worldX(float tileX) const { return tileX * _tileSize; }

float MapCamera::worldY(float tileY) const { return tileY * _tileSize; }

bool MapCamera::isTileVisible(int tileX, int tileY) const
{
    return tileX >= static_cast<int>(_originX) &&
           tileX < static_cast<int>(_originX) + _viewTilesX &&
           tileY >= static_cast<int>(_originY) &&
           tileY < static_cast<int>(_originY) + _viewTilesY;
}

int MapCamera::tileXFromWorld(float wx) const
{
    return static_cast<int>(std::floor(wx / _tileSize));
}

int MapCamera::tileYFromWorld(float wy) const
{
    return static_cast<int>(std::floor(wy / _tileSize));
}
