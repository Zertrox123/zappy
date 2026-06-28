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

sf::Vector2f effectCenter(const WorldEffect &effect, const MapCamera &camera)
{
    return {camera.worldX(effect.x) + camera.tileSize() * 0.5f,
            camera.worldY(effect.y) + camera.tileSize() * 0.5f};
}
} // namespace

void EffectRenderer::drawExpulsion(sf::RenderWindow &window,
                                   const WorldEffect &effect,
                                   const MapCamera &camera) const
{
    const sf::Vector2f center = effectCenter(effect, camera);
    const float tile = camera.tileSize();
    const float radius = tile * 0.15f + effect.age * tile * 1.2f;
    const auto alpha =
        static_cast<sf::Uint8>(std::max(0.f, 220.f - effect.age * 360.f));

    sf::CircleShape ring(radius);
    ring.setOrigin(radius, radius);
    ring.setPosition(center);
    ring.setFillColor(sf::Color::Transparent);
    ring.setOutlineColor(sf::Color(255, 140, 40, alpha));
    ring.setOutlineThickness(std::max(2.f, tile * 0.06f));
    window.draw(ring);
}

void EffectRenderer::drawBroadcast(sf::RenderWindow &window,
                                   const WorldEffect &effect,
                                   const PlayerAnimator &animator,
                                   const MapCamera &camera) const
{
    const float tile = camera.tileSize();
    PlayerAnimator::Snapshot snap{};
    sf::Vector2f anchor = effectCenter(effect, camera);
    if (animator.snapshot(effect.playerId, snap))
    {
        anchor = {camera.worldX(snap.x) + tile * 0.5f,
                  camera.worldY(snap.y) + tile * 0.2f};
    }

    const float rise = effect.age * tile * 0.55f;
    const auto alpha =
        static_cast<sf::Uint8>(std::max(0.f, 255.f - effect.age * 85.f));

    sf::RectangleShape bubble(sf::Vector2f(tile * 3.6f, tile * 0.85f));
    bubble.setFillColor(sf::Color(24, 28, 36, alpha));
    bubble.setOutlineColor(sf::Color(255, 255, 255, alpha));
    bubble.setOutlineThickness(1.f);
    bubble.setPosition(anchor.x - tile * 1.8f, anchor.y - rise - tile);
    window.draw(bubble);

    if (!UiFont::available() || effect.message.empty())
        return;

    sf::Text text(effect.message, UiFont::get(),
                  static_cast<unsigned int>(std::max(11.f, tile * 0.32f)));
    text.setFillColor(sf::Color(255, 255, 255, alpha));
    text.setPosition(anchor.x - tile * 1.7f, anchor.y - rise - tile * 0.9f);
    window.draw(text);
}

void EffectRenderer::drawIncantation(sf::RenderWindow &window,
                                     const WorldEffect &effect,
                                     const MapCamera &camera) const
{
    const sf::Vector2f center = effectCenter(effect, camera);
    const float tile = camera.tileSize();
    const float pulse =
        tile * 0.35f + std::sin(effect.age * 6.f) * tile * 0.08f;
    const auto alpha =
        static_cast<sf::Uint8>(150 + std::sin(effect.age * 4.f) * 80);

    sf::CircleShape aura(pulse);
    aura.setOrigin(pulse, pulse);
    aura.setPosition(center);
    aura.setFillColor(
        sf::Color(140, 80, 220, static_cast<sf::Uint8>(alpha / 3)));
    aura.setOutlineColor(sf::Color(200, 140, 255, alpha));
    aura.setOutlineThickness(std::max(2.f, tile * 0.05f));
    window.draw(aura);
}

void EffectRenderer::drawFork(sf::RenderWindow &window,
                              const WorldEffect &effect,
                              const MapCamera &camera) const
{
    const sf::Vector2f center = effectCenter(effect, camera);
    const float tile = camera.tileSize();
    const float size = tile * 0.18f + effect.age * tile * 0.25f;
    const auto alpha =
        static_cast<sf::Uint8>(std::max(0.f, 220.f - effect.age * 180.f));

    sf::CircleShape egg(size);
    egg.setOrigin(size, size);
    egg.setPosition(center);
    egg.setFillColor(sf::Color(240, 230, 120, alpha));
    egg.setOutlineColor(sf::Color::White);
    egg.setOutlineThickness(1.f);
    window.draw(egg);
}

void EffectRenderer::drawResourceFx(sf::RenderWindow &window,
                                    const WorldEffect &effect,
                                    const MapCamera &camera, bool drop) const
{
    const sf::Vector2f center = effectCenter(effect, camera);
    const float tile = camera.tileSize();
    const int idx = std::clamp(effect.resource, 0, 6);
    const float offset =
        drop ? effect.age * tile * 0.5f : -effect.age * tile * 0.5f;
    const auto alpha =
        static_cast<sf::Uint8>(std::max(0.f, 220.f - effect.age * 260.f));

    sf::CircleShape dot(tile * 0.12f);
    dot.setFillColor(sf::Color(kResourceColors[idx].r, kResourceColors[idx].g,
                               kResourceColors[idx].b, alpha));
    dot.setPosition(center.x - tile * 0.12f, center.y + offset - tile * 0.12f);
    window.draw(dot);
}

void EffectRenderer::drawIncantationEnd(sf::RenderWindow &window,
                                        const WorldEffect &effect,
                                        const MapCamera &camera) const
{
    const sf::Vector2f center = effectCenter(effect, camera);
    const float tile = camera.tileSize();
    const float radius = tile * 0.2f + effect.age * tile * 0.7f;
    const auto alpha =
        static_cast<sf::Uint8>(std::max(0.f, 220.f - effect.age * 140.f));
    const sf::Color color = effect.success ? sf::Color(80, 220, 120, alpha)
                                           : sf::Color(220, 80, 80, alpha);

    sf::CircleShape flash(radius);
    flash.setOrigin(radius, radius);
    flash.setPosition(center);
    flash.setFillColor(sf::Color::Transparent);
    flash.setOutlineColor(color);
    flash.setOutlineThickness(std::max(2.f, tile * 0.05f));
    window.draw(flash);
}

void EffectRenderer::drawDeath(sf::RenderWindow &window,
                               const WorldEffect &effect,
                               const MapCamera &camera) const
{
    const sf::Vector2f center = effectCenter(effect, camera);
    const float tile = camera.tileSize();
    const auto alpha =
        static_cast<sf::Uint8>(std::max(0.f, 220.f - effect.age * 220.f));

    sf::RectangleShape cross(sf::Vector2f(tile * 0.45f, tile * 0.07f));
    cross.setFillColor(sf::Color(220, 60, 60, alpha));
    cross.setOrigin(tile * 0.225f, tile * 0.035f);
    cross.setPosition(center);
    cross.setRotation(45.f);
    window.draw(cross);

    sf::RectangleShape cross2(sf::Vector2f(tile * 0.45f, tile * 0.07f));
    cross2.setFillColor(sf::Color(220, 60, 60, alpha));
    cross2.setOrigin(tile * 0.225f, tile * 0.035f);
    cross2.setPosition(center);
    cross2.setRotation(-45.f);
    window.draw(cross2);
}

void EffectRenderer::draw(sf::RenderWindow &window, const GameState &state,
                          const PlayerAnimator &animator,
                          const MapCamera &camera) const
{
    for (const WorldEffect &effect : state.effects())
    {
        if (!camera.isTileVisible(effect.x, effect.y) &&
            effect.kind != EffectKind::Broadcast)
            continue;

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
