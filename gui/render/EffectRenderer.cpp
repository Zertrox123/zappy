#include "render/EffectRenderer.hpp"
#include "render/UiFont.hpp"

#include <cmath>

void EffectRenderer::drawExpulsion(sf::RenderWindow &window,
                                   const WorldEffect &effect) const
{
    const float cx = static_cast<float>(effect.x * kTileSize + 16);
    const float cy = static_cast<float>(effect.y * kTileSize + 16);
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
                                   const PlayerAnimator &animator) const
{
    PlayerAnimator::Snapshot snap{};
    float cx = static_cast<float>(effect.x * kTileSize + 16);
    float cy = static_cast<float>(effect.y * kTileSize + 8);
    if (animator.snapshot(effect.playerId, snap))
    {
        cx = snap.x * static_cast<float>(kTileSize) + 16.f;
        cy = snap.y * static_cast<float>(kTileSize) + 4.f;
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
                                     const WorldEffect &effect) const
{
    const float cx = static_cast<float>(effect.x * kTileSize + 16);
    const float cy = static_cast<float>(effect.y * kTileSize + 16);
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

void EffectRenderer::draw(sf::RenderWindow &window, const GameState &state,
                          const PlayerAnimator &animator) const
{
    for (const WorldEffect &effect : state.effects())
    {
        switch (effect.kind)
        {
        case EffectKind::Expulsion:
            drawExpulsion(window, effect);
            break;
        case EffectKind::Broadcast:
            drawBroadcast(window, effect, animator);
            break;
        case EffectKind::Incantation:
            drawIncantation(window, effect);
            break;
        }
    }
}
