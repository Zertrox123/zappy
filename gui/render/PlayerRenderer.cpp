#include "render/PlayerRenderer.hpp"
#include "render/UiFont.hpp"

#include <algorithm>
#include <cmath>

namespace
{
sf::Vector2f facingOffset(int orientation, float distance)
{
    switch (orientation)
    {
    case 1:
        return {0.f, -distance};
    case 2:
        return {distance, 0.f};
    case 3:
        return {0.f, distance};
    case 4:
        return {-distance, 0.f};
    default:
        return {0.f, -distance};
    }
}
} // namespace

void PlayerRenderer::draw(sf::RenderWindow &window,
                          const PlayerAnimator::Snapshot &snap,
                          const sf::Color &color, float tileSize)
{
    const float cx = snap.x * tileSize + tileSize * 0.5f;
    const float cy = snap.y * tileSize + tileSize * 0.5f;
    const float bodyR = tileSize * 0.24f;

    sf::CircleShape shadow(bodyR);
    shadow.setScale(1.35f, 0.5f);
    shadow.setOrigin(bodyR, bodyR);
    shadow.setPosition(cx, cy + tileSize * 0.18f);
    shadow.setFillColor(sf::Color(0, 0, 0, 65));
    window.draw(shadow);

    sf::CircleShape ring(bodyR + tileSize * 0.04f);
    ring.setOrigin(bodyR + tileSize * 0.04f, bodyR + tileSize * 0.04f);
    ring.setPosition(cx, cy);
    ring.setFillColor(sf::Color::Transparent);
    ring.setOutlineColor(sf::Color(255, 255, 255, 90));
    ring.setOutlineThickness(std::max(1.f, tileSize * 0.03f));
    window.draw(ring);

    sf::CircleShape body(bodyR);
    body.setOrigin(bodyR, bodyR);
    body.setPosition(cx, cy);
    body.setFillColor(color);
    body.setOutlineColor(sf::Color(255, 255, 255, 230));
    body.setOutlineThickness(std::max(1.5f, tileSize * 0.035f));
    window.draw(body);

    const sf::Vector2f lightOff(-bodyR * 0.28f, -bodyR * 0.28f);
    sf::CircleShape highlight(bodyR * 0.28f);
    highlight.setOrigin(bodyR * 0.28f, bodyR * 0.28f);
    highlight.setPosition(cx + lightOff.x, cy + lightOff.y);
    highlight.setFillColor(sf::Color(255, 255, 255, 70));
    highlight.setOutlineThickness(0.f);
    window.draw(highlight);

    const sf::Vector2f nose = facingOffset(snap.orientation, bodyR * 0.95f);
    sf::CircleShape marker(tileSize * 0.07f);
    marker.setOrigin(tileSize * 0.07f, tileSize * 0.07f);
    marker.setPosition(cx + nose.x, cy + nose.y);
    marker.setFillColor(sf::Color(255, 255, 255, 240));
    marker.setOutlineColor(sf::Color(20, 24, 30, 200));
    marker.setOutlineThickness(std::max(1.f, tileSize * 0.015f));
    window.draw(marker);

    if (UiFont::available())
    {
        const unsigned char fontSize = static_cast<unsigned char>(
            std::clamp(tileSize * 0.28f, 10.f, 15.f));
        sf::Text level(std::to_string(snap.level), UiFont::get(), fontSize);
        level.setFillColor(sf::Color(255, 255, 255, 245));
        level.setOutlineColor(sf::Color(0, 0, 0, 190));
        level.setOutlineThickness(1.f);
        const auto bounds = level.getLocalBounds();
        level.setOrigin(bounds.width / 2.f, bounds.height);
        level.setPosition(cx, cy - bodyR - tileSize * 0.06f);
        window.draw(level);
    }
}
