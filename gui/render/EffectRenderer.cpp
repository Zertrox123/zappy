#include "render/EffectRenderer.hpp"
#include "render/UiFont.hpp"

#include <algorithm>
#include <cmath>

namespace
{
const sf::Color kResourceColors[7] = {
    sf::Color(240, 220, 80),  sf::Color(180, 180, 180),
    sf::Color(200, 120, 60),  sf::Color(120, 200, 255),
    sf::Color(120, 220, 120), sf::Color(220, 120, 220),
    sf::Color(160, 120, 255),
};

float tileCenterX(const WorldEffect &effect, const MapCamera &camera,
                  unsigned tileSize)
{
    return camera.screenX(effect.x, tileSize) +
           static_cast<float>(tileSize) / 2.f;
}

float tileCenterY(const WorldEffect &effect, const MapCamera &camera,
                  unsigned tileSize)
{
    return camera.screenY(effect.y, tileSize) +
           static_cast<float>(tileSize) / 2.f;
}
} // namespace

void EffectRenderer::drawExpulsion(sf::RenderWindow &window,
                                   const WorldEffect &effect,
                                   const MapCamera &camera) const
{
    const float cx = tileCenterX(effect, camera, kTileSize);
    const float cy = tileCenterY(effect, camera, kTileSize);
    const float radius = 8.f + effect.age * 40.f;
    const auto alpha =
        static_cast<sf::Uint8>(std::max(0.f, 220.f - effect.age * 360.f));

    sf::CircleShape ring(radius);
    ring.setOrigin(radius, radius);
    ring.setPosition(cx, cy);
    ring.setFillColor(sf::Color::Transparent);
    ring.setOutlineColor(sf::Color(255, 140, 40, alpha));
    ring.setOutlineThickness(3.f);
    window.draw(ring);
}

void EffectRenderer::drawBroadcast(sf::RenderWindow &window,
                                   const WorldEffect &effect,
                                   const PlayerAnimator &animator,
                                   const MapCamera &camera) const
{
    PlayerAnimator::Snapshot snap{};
    float cx = tileCenterX(effect, camera, kTileSize);
    float cy = camera.screenY(effect.y, kTileSize) + 8.f;
    if (animator.snapshot(effect.playerId, snap))
    {
        cx = camera.screenX(static_cast<int>(snap.x + 0.5f), kTileSize) + 16.f;
        cy = camera.screenY(static_cast<int>(snap.y + 0.5f), kTileSize) + 4.f;
    }

    const float rise = effect.age * 18.f;
    const auto alpha =
        static_cast<sf::Uint8>(std::max(0.f, 255.f - effect.age * 85.f));

    sf::RectangleShape bubble(sf::Vector2f(120.f, 28.f));
    bubble.setFillColor(sf::Color(30, 30, 40, alpha));
    bubble.setOutlineColor(sf::Color(255, 255, 255, alpha));
    bubble.setOutlineThickness(1.f);
    bubble.setPosition(cx - 60.f, cy - rise - 30.f);
    window.draw(bubble);

    if (!UiFont::available() || effect.message.empty())
        return;

    sf::Text text(effect.message, UiFont::get(), 12);
    text.setFillColor(sf::Color(255, 255, 255, alpha));
    text.setPosition(cx - 56.f, cy - rise - 26.f);
    window.draw(text);
}

void EffectRenderer::drawIncantation(sf::RenderWindow &window,
                                     const WorldEffect &effect,
                                     const MapCamera &camera) const
{
    const float cx = tileCenterX(effect, camera, kTileSize);
    const float cy = tileCenterY(effect, camera, kTileSize);
    const float pulse = 14.f + std::sin(effect.age * 6.f) * 4.f;
    const auto alpha =
        static_cast<sf::Uint8>(150 + std::sin(effect.age * 4.f) * 80);

    sf::CircleShape aura(pulse);
    aura.setOrigin(pulse, pulse);
    aura.setPosition(cx, cy);
    aura.setFillColor(
        sf::Color(140, 80, 220, static_cast<sf::Uint8>(alpha / 3)));
    aura.setOutlineColor(sf::Color(200, 140, 255, alpha));
    aura.setOutlineThickness(2.f);
    window.draw(aura);
}

void EffectRenderer::drawFork(sf::RenderWindow &window,
                              const WorldEffect &effect,
                              const MapCamera &camera) const
{
    const float cx = tileCenterX(effect, camera, kTileSize);
    const float cy = tileCenterY(effect, camera, kTileSize);
    const float size = 8.f + effect.age * 10.f;
    const auto alpha =
        static_cast<sf::Uint8>(std::max(0.f, 220.f - effect.age * 180.f));

    sf::CircleShape egg(size);
    egg.setOrigin(size, size);
    egg.setPosition(cx, cy);
    egg.setFillColor(sf::Color(240, 230, 120, alpha));
    egg.setOutlineColor(sf::Color::White);
    egg.setOutlineThickness(1.f);
    window.draw(egg);
}

void EffectRenderer::drawResourceFx(sf::RenderWindow &window,
                                    const WorldEffect &effect,
                                    const MapCamera &camera, bool drop) const
{
    const float cx = tileCenterX(effect, camera, kTileSize);
    const float cy = tileCenterY(effect, camera, kTileSize);
    const int idx = std::clamp(effect.resource, 0, 6);
    const float offset = drop ? effect.age * 16.f : -effect.age * 16.f;
    const auto alpha =
        static_cast<sf::Uint8>(std::max(0.f, 220.f - effect.age * 260.f));

    sf::CircleShape dot(5.f);
    dot.setFillColor(sf::Color(kResourceColors[idx].r, kResourceColors[idx].g,
                               kResourceColors[idx].b, alpha));
    dot.setPosition(cx - 5.f, cy + offset - 5.f);
    window.draw(dot);
}

void EffectRenderer::drawIncantationEnd(sf::RenderWindow &window,
                                        const WorldEffect &effect,
                                        const MapCamera &camera) const
{
    const float cx = tileCenterX(effect, camera, kTileSize);
    const float cy = tileCenterY(effect, camera, kTileSize);
    const float radius = 10.f + effect.age * 24.f;
    const auto alpha =
        static_cast<sf::Uint8>(std::max(0.f, 220.f - effect.age * 140.f));
    const sf::Color color = effect.success ? sf::Color(80, 220, 120, alpha)
                                           : sf::Color(220, 80, 80, alpha);

    sf::CircleShape flash(radius);
    flash.setOrigin(radius, radius);
    flash.setPosition(cx, cy);
    flash.setFillColor(sf::Color::Transparent);
    flash.setOutlineColor(color);
    flash.setOutlineThickness(3.f);
    window.draw(flash);
}

void EffectRenderer::drawDeath(sf::RenderWindow &window,
                               const WorldEffect &effect,
                               const MapCamera &camera) const
{
    const float cx = tileCenterX(effect, camera, kTileSize);
    const float cy = tileCenterY(effect, camera, kTileSize);
    const auto alpha =
        static_cast<sf::Uint8>(std::max(0.f, 220.f - effect.age * 220.f));

    sf::RectangleShape cross(sf::Vector2f(18.f, 3.f));
    cross.setFillColor(sf::Color(220, 60, 60, alpha));
    cross.setOrigin(9.f, 1.5f);
    cross.setPosition(cx, cy);
    cross.setRotation(45.f);
    window.draw(cross);

    sf::RectangleShape cross2(sf::Vector2f(18.f, 3.f));
    cross2.setFillColor(sf::Color(220, 60, 60, alpha));
    cross2.setOrigin(9.f, 1.5f);
    cross2.setPosition(cx, cy);
    cross2.setRotation(-45.f);
    window.draw(cross2);
}

void EffectRenderer::draw(sf::RenderWindow &window, const GameState &state,
                          const PlayerAnimator &animator,
                          const MapCamera &camera) const
{
    for (const WorldEffect &effect : state.effects())
    {
        switch (effect.kind)
        {
        case EffectKind::Expulsion:
            drawExpulsion(window, effect, camera);
            break;
        case EffectKind::Broadcast:
            drawBroadcast(window, effect, animator, camera);
            break;
        case EffectKind::Incantation:
            drawIncantation(window, effect, camera);
            break;
        case EffectKind::Fork:
            drawFork(window, effect, camera);
            break;
        case EffectKind::ResourceDrop:
            drawResourceFx(window, effect, camera, true);
            break;
        case EffectKind::ResourceTake:
            drawResourceFx(window, effect, camera, false);
            break;
        case EffectKind::IncantationEnd:
            drawIncantationEnd(window, effect, camera);
            break;
        case EffectKind::Death:
            drawDeath(window, effect, camera);
            break;
        }
    }
}
