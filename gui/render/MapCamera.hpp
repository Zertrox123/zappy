#pragma once

#include <SFML/Graphics.hpp>

class MapCamera
{
  public:
    void fitToViewport(unsigned pixelWidth, unsigned pixelHeight, int mapWidth,
                       int mapHeight);
    void pan(float deltaTilesX, float deltaTilesY);
    void zoomIn();
    void zoomOut();
    void applyToView(sf::View &view, unsigned mapPixelWidth,
                     unsigned mapPixelHeight, unsigned windowWidth,
                     unsigned windowHeight) const;

    float originX() const { return _originX; }
    float originY() const { return _originY; }
    int viewTilesX() const { return _viewTilesX; }
    int viewTilesY() const { return _viewTilesY; }
    float tileSize() const { return _tileSize; }

    float worldX(int tileX) const;
    float worldY(int tileY) const;
    float worldX(float tileX) const;
    float worldY(float tileY) const;
    bool isTileVisible(int tileX, int tileY) const;
    int tileXFromWorld(float worldX) const;
    int tileYFromWorld(float worldY) const;

  private:
    static constexpr float kTargetTileSize = 44.f;
    static constexpr float kMinTileSize = 20.f;
    static constexpr float kMaxTileSize = 120.f;

    int _mapWidth = 0;
    int _mapHeight = 0;
    int _viewTilesX = 0;
    int _viewTilesY = 0;
    unsigned _pixelWidth = 0;
    unsigned _pixelHeight = 0;
    int _aspectNumerator = 1;
    int _aspectDenominator = 1;
    int _scale = 0;
    int _minScale = 1;
    int _maxScale = 1;
    float _originX = 0.f;
    float _originY = 0.f;
    float _tileSize = 40.f;

    void updateScaleLimits();
    void rebuildView();
    void clampOrigin();
};
