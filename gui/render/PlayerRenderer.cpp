#include "render/PlayerRenderer.hpp"
#include "render/UiFont.hpp"

void PlayerRenderer::draw(sf::RenderWindow &window,
                          const PlayerAnimator::Snapshot &snap,
                          const sf::Color &color)
{
    const float cx = snap.x * static_cast<float>(kTileSize) + 16.f;
    const float cy = snap.y * static_cast<float>(kTileSize) + 16.f;

    sf::ConvexShape body(3);
    body.setPoint(0, {0.f, -14.f});
    body.setPoint(1, {12.f, 10.f});
    body.setPoint(2, {-12.f, 10.f});
    body.setFillColor(color);
    body.setOutlineColor(sf::Color::White);
    body.setOutlineThickness(2.f);

    float rotation = 0.f;
    switch (snap.orientation)
    {
    case 1:
        rotation = 0.f;
        break;
    case 2:
        rotation = 90.f;
        break;
    case 3:
        rotation = 180.f;
        break;
    case 4:
        rotation = -90.f;
        break;
    default:
        break;
    }
    body.setRotation(rotation);
    body.setPosition(cx, cy);
    window.draw(body);

    if (UiFont::available())
    {
        sf::Text level(std::to_string(snap.level), UiFont::get(), 11);
        level.setFillColor(sf::Color::White);
        level.setOutlineColor(sf::Color::Black);
        level.setOutlineThickness(1.f);
        level.setPosition(cx - 4.f, cy - 24.f);
        window.draw(level);
    }
}
