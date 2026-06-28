#include "render/MapIcons.hpp"
#include "model/Tile.hpp"
#include "render/UiFont.hpp"

#include <algorithm>
#include <cmath>
#include <vector>

namespace
{
float iconRadius(float tileSize) { return tileSize * 0.13f; }

void centerShape(sf::Shape &shape, float cx, float cy)
{
    const sf::FloatRect bounds = shape.getLocalBounds();
    shape.setOrigin(bounds.left + bounds.width / 2.f,
                    bounds.top + bounds.height / 2.f);
    shape.setPosition(cx, cy);
}

void drawOutlined(sf::RenderWindow &window, sf::Shape &shape,
                  const sf::Color &fill, const sf::Color &outline,
                  float thickness)
{
    shape.setFillColor(fill);
    shape.setOutlineColor(outline);
    shape.setOutlineThickness(thickness);
    window.draw(shape);
}

void drawFood(sf::RenderWindow &window, float cx, float cy, float r,
              const MapIcons::ResourceStyle &style)
{
    sf::CircleShape body(r);
    centerShape(body, cx, cy);
    drawOutlined(window, body, style.fill, style.outline,
                 std::max(1.f, r * 0.12f));

    sf::CircleShape shine(r * 0.35f);
    centerShape(shine, cx - r * 0.25f, cy - r * 0.25f);
    shine.setFillColor(style.highlight);
    shine.setOutlineThickness(0.f);
    window.draw(shine);
}

void drawHexStone(sf::RenderWindow &window, float cx, float cy, float r,
                  const MapIcons::ResourceStyle &style)
{
    sf::ConvexShape hex(6);
    for (int i = 0; i < 6; ++i)
    {
        const float angle =
            static_cast<float>(i) * 3.14159265f / 3.f - 3.14159265f / 6.f;
        hex.setPoint(i, {std::cos(angle) * r, std::sin(angle) * r});
    }
    centerShape(hex, cx, cy);
    drawOutlined(window, hex, style.fill, style.outline,
                 std::max(1.f, r * 0.1f));
}

void drawShard(sf::RenderWindow &window, float cx, float cy, float r,
               const MapIcons::ResourceStyle &style)
{
    sf::ConvexShape shard(4);
    shard.setPoint(0, {0.f, -r});
    shard.setPoint(1, {r * 0.55f, 0.f});
    shard.setPoint(2, {0.f, r * 0.85f});
    shard.setPoint(3, {-r * 0.55f, 0.f});
    centerShape(shard, cx, cy);
    drawOutlined(window, shard, style.fill, style.outline,
                 std::max(1.f, r * 0.1f));
}

void drawDroplet(sf::RenderWindow &window, float cx, float cy, float r,
                 const MapIcons::ResourceStyle &style)
{
    sf::CircleShape drop(r);
    drop.setScale(0.8f, 1.05f);
    centerShape(drop, cx, cy + r * 0.05f);
    drawOutlined(window, drop, style.fill, style.outline,
                 std::max(1.f, r * 0.1f));

    sf::CircleShape shine(r * 0.22f);
    centerShape(shine, cx - r * 0.2f, cy - r * 0.15f);
    shine.setFillColor(style.highlight);
    shine.setOutlineThickness(0.f);
    window.draw(shine);
}

void drawLeaf(sf::RenderWindow &window, float cx, float cy, float r,
              const MapIcons::ResourceStyle &style)
{
    sf::ConvexShape leaf(5);
    leaf.setPoint(0, {0.f, -r});
    leaf.setPoint(1, {r * 0.75f, -r * 0.1f});
    leaf.setPoint(2, {r * 0.35f, r});
    leaf.setPoint(3, {-r * 0.35f, r});
    leaf.setPoint(4, {-r * 0.75f, -r * 0.1f});
    centerShape(leaf, cx, cy);
    drawOutlined(window, leaf, style.fill, style.outline,
                 std::max(1.f, r * 0.1f));
}

void drawStar(sf::RenderWindow &window, float cx, float cy, float r,
              const MapIcons::ResourceStyle &style)
{
    sf::ConvexShape star(8);
    for (int i = 0; i < 8; ++i)
    {
        const float angle = static_cast<float>(i) * 3.14159265f / 4.f;
        const float radius = (i % 2 == 0) ? r : r * 0.45f;
        star.setPoint(i, {std::cos(angle) * radius, std::sin(angle) * radius});
    }
    centerShape(star, cx, cy);
    drawOutlined(window, star, style.fill, style.outline,
                 std::max(1.f, r * 0.1f));
}

void drawCrystal(sf::RenderWindow &window, float cx, float cy, float r,
                 const MapIcons::ResourceStyle &style)
{
    sf::ConvexShape crystal(5);
    crystal.setPoint(0, {0.f, -r * 1.1f});
    crystal.setPoint(1, {r * 0.55f, -r * 0.1f});
    crystal.setPoint(2, {r * 0.35f, r});
    crystal.setPoint(3, {-r * 0.35f, r});
    crystal.setPoint(4, {-r * 0.55f, -r * 0.1f});
    centerShape(crystal, cx, cy);
    drawOutlined(window, crystal, style.fill, style.outline,
                 std::max(1.f, r * 0.1f));

    sf::RectangleShape facet(sf::Vector2f(r * 0.12f, r * 0.7f));
    facet.setFillColor(style.highlight);
    centerShape(facet, cx, cy);
    window.draw(facet);
}

void drawQuantityBadge(sf::RenderWindow &window, float cx, float cy, float r,
                       int quantity)
{
    if (quantity <= 1 || !UiFont::available())
        return;

    sf::CircleShape badge(r * 0.42f);
    centerShape(badge, cx + r * 0.55f, cy + r * 0.45f);
    badge.setFillColor(sf::Color(18, 22, 28, 190));
    badge.setOutlineColor(sf::Color(255, 255, 255, 140));
    badge.setOutlineThickness(1.f);
    window.draw(badge);

    sf::Text label(std::to_string(quantity), UiFont::get(),
                   static_cast<unsigned int>(std::max(7.f, r * 0.65f)));
    label.setFillColor(sf::Color::White);
    const auto bounds = label.getLocalBounds();
    label.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
    label.setPosition(cx + r * 0.55f, cy + r * 0.43f);
    window.draw(label);
}
} // namespace

namespace MapIcons
{
ResourceStyle resourceStyle(int resourceType)
{
    static const ResourceStyle styles[7] = {
        {sf::Color(255, 208, 72), sf::Color(255, 236, 150),
         sf::Color(120, 86, 20)},
        {sf::Color(168, 174, 184), sf::Color(220, 224, 232),
         sf::Color(72, 78, 88)},
        {sf::Color(214, 132, 72), sf::Color(244, 180, 120),
         sf::Color(110, 62, 28)},
        {sf::Color(92, 176, 255), sf::Color(180, 220, 255),
         sf::Color(28, 84, 140)},
        {sf::Color(104, 210, 118), sf::Color(180, 240, 188),
         sf::Color(36, 100, 48)},
        {sf::Color(236, 132, 196), sf::Color(255, 196, 230),
         sf::Color(120, 48, 92)},
        {sf::Color(164, 124, 255), sf::Color(214, 190, 255),
         sf::Color(72, 44, 140)},
    };
    const int idx = std::clamp(resourceType, 0, 6);
    return styles[idx];
}

sf::Vector2f resourceSlot(int resourceType, float tileSize, float originX,
                          float originY)
{
    static const sf::Vector2f slots[7] = {
        {0.50f, 0.24f}, {0.76f, 0.34f}, {0.80f, 0.58f}, {0.64f, 0.78f},
        {0.36f, 0.78f}, {0.20f, 0.58f}, {0.24f, 0.34f},
    };
    const int idx = std::clamp(resourceType, 0, 6);
    return {originX + slots[idx].x * tileSize,
            originY + slots[idx].y * tileSize};
}

void drawResource(sf::RenderWindow &window, int resourceType, float centerX,
                  float centerY, float tileSize, int quantity)
{
    const float r = iconRadius(tileSize);
    const ResourceStyle style = resourceStyle(resourceType);

    switch (resourceType)
    {
    case 0:
        drawFood(window, centerX, centerY, r, style);
        break;
    case 1:
        drawHexStone(window, centerX, centerY, r, style);
        break;
    case 2:
        drawShard(window, centerX, centerY, r, style);
        break;
    case 3:
        drawDroplet(window, centerX, centerY, r, style);
        break;
    case 4:
        drawLeaf(window, centerX, centerY, r, style);
        break;
    case 5:
        drawStar(window, centerX, centerY, r, style);
        break;
    default:
        drawCrystal(window, centerX, centerY, r, style);
        break;
    }

    drawQuantityBadge(window, centerX, centerY, r, quantity);
}

void drawTileResources(sf::RenderWindow &window, const Tile &tile,
                       float originX, float originY, float tileSize)
{
    std::vector<int> types;
    types.reserve(7);
    for (int resource = 0; resource < 7; ++resource)
    {
        if (tile.resources[resource] > 0)
            types.push_back(resource);
    }
    if (types.empty())
        return;

    const float centerX = originX + tileSize * 0.5f;
    const float centerY = originY + tileSize * 0.5f;
    const float scale =
        types.size() == 1 ? 1.15f : (types.size() <= 3 ? 1.f : 0.88f);
    const float layoutTile = tileSize * scale;
    const float ring = types.size() == 1 ? 0.f : tileSize * 0.2f;

    for (std::size_t i = 0; i < types.size(); ++i)
    {
        const float angle =
            -3.14159265f / 2.f + static_cast<float>(i) * 6.2831853f /
                                     static_cast<float>(types.size());
        const float cx = centerX + std::cos(angle) * ring;
        const float cy = centerY + std::sin(angle) * ring;
        drawResource(window, types[i], cx, cy, layoutTile,
                     tile.resources[types[i]]);
    }
}

void drawEgg(sf::RenderWindow &window, float centerX, float centerY,
             float tileSize)
{
    const float r = tileSize * 0.16f;

    sf::CircleShape shadow(r);
    shadow.setScale(1.2f, 0.45f);
    centerShape(shadow, centerX, centerY + tileSize * 0.18f);
    shadow.setFillColor(sf::Color(0, 0, 0, 55));
    window.draw(shadow);

    sf::CircleShape shell(r);
    shell.setScale(0.82f, 1.f);
    centerShape(shell, centerX, centerY);
    shell.setFillColor(sf::Color(248, 232, 148));
    shell.setOutlineColor(sf::Color(255, 255, 255, 210));
    shell.setOutlineThickness(std::max(1.f, tileSize * 0.025f));
    window.draw(shell);

    sf::CircleShape speckle(r * 0.22f);
    centerShape(speckle, centerX - r * 0.22f, centerY - r * 0.28f);
    speckle.setFillColor(sf::Color(255, 255, 255, 150));
    speckle.setOutlineThickness(0.f);
    window.draw(speckle);
}

} // namespace MapIcons
