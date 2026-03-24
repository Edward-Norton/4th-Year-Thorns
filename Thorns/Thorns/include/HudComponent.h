#ifndef HUD_COMPONENT_H
#define HUD_COMPONENT_H

#include <SFML/Graphics.hpp>
#include <string>
#include "IRenderable.h"
#include "HealthComponent.h"
#include "StatComponent.h"

/// <summary>
/// Render HUD elements (health bar, stamina, etc.
/// 
/// Note of Bar rendering order:
///     HP          (RED)
///     STAMINA     (WHITE)
///     FOOD        (ORANGE)
///     WATER       (BLUE)
/// </summary>
class HUDComponent : public IRenderable
{
public:
    explicit HUDComponent(const HealthComponent& health,
                          const StatComponent& stamina, 
                          const StatComponent& hunger, 
                          const StatComponent& water); // Just to ensure no swap conversion, 
    HUDComponent() = default;
    ~HUDComponent() = default;

    // ========== Initialization ==========
    // font paths
    bool initialize(const std::string& fontPath);

    // ========== IRenderable ==========
    void render(sf::RenderTarget& target) const override;

private:
    // Simple basic bar to display a UI element with text. 
    void drawBar(sf::RenderTarget& target,
        float screenY,
        float ratio,
        const sf::Color& fillColor,
        const std::string& label,
        float current,
        float max) const;

    // Data to show
    const HealthComponent& m_health;
    const StatComponent& m_stamina;
    const StatComponent& m_hunger;
    const StatComponent& m_water;

    // ========== Font ==========
    sf::Font m_font;
    bool m_fontLoaded;

    // ========== Layout constants ==========
    // All values are in screen-space pixels.
    static constexpr float BAR_X = 40.f;   // Left edge of bar area
    static constexpr float BAR_Y = 20.f;   // Top edge of first bar
    static constexpr float BAR_WIDTH = 200.f;  // Full bar width
    static constexpr float BAR_HEIGHT = 18.f;   // Bar height
    static constexpr float LABEL_OFFSET_X = -4.f;   // Label sits left of bar (drawn right-to-left)
    static constexpr float BAR_SPACING = 30.f;   // Vertical gap between stacked bars

    // ========== Colors ==========
    static const sf::Color COLOR_BAR_BACKGROUND;
    static const sf::Color COLOR_BAR_OUTLINE;
    static const sf::Color COLOR_HEALTH_FILL;   // Red
    static const sf::Color COLOR_STAMINA_FILL;  // White
    static const sf::Color COLOR_HUNGER_FILL;   // Orange
    static const sf::Color COLOR_WATER_FILL;    // Blue
    static const sf::Color COLOR_TEXT;
};

#endif