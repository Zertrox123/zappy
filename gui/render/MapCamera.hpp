#pragma once

class MapCamera
{
  public:
    void configure(int mapWidth, int mapHeight, int viewTilesX, int viewTilesY);
    void pan(int deltaTilesX, int deltaTilesY);

    int originX() const { return _originX; }
    int originY() const { return _originY; }
    int viewTilesX() const { return _viewTilesX; }
    int viewTilesY() const { return _viewTilesY; }

    float screenX(int tileX, unsigned tileSize) const;
    float screenY(int tileY, unsigned tileSize) const;
    int tileXFromScreen(int pixelX, unsigned tileSize) const;
    int tileYFromScreen(int pixelY, unsigned tileSize) const;

  private:
    int _mapWidth = 0;
    int _mapHeight = 0;
    int _viewTilesX = 0;
    int _viewTilesY = 0;
    int _originX = 0;
    int _originY = 0;
};
