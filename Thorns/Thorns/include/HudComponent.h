#ifndef HUD_COMPONENT_H
#define HUD_COMPONENT_H

#include <SFML/Graphics.hpp>
#include <string>
#include "IRenderable.h"

/// <summary>
/// Render HUD elements (health bar, stamina, etc.
/// </summary>
class HUDComponent : public IRenderable
{
public:
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

    // ========== Font ==========
    sf::Font m_font;
    bool m_fontLoaded;

    // ========== Layout constants ==========
    // All values are in screen-space pixels.
    static constexpr float BAR_X = 20.f;   // Left edge of bar area
    static constexpr float BAR_Y = 20.f;   // Top edge of first bar
    static constexpr float BAR_WIDTH = 200.f;  // Full bar width
    static constexpr float BAR_HEIGHT = 18.f;   // Bar height
    static constexpr float LABEL_OFFSET_X = -4.f;   // Label sits left of bar (drawn right-to-left)
    static constexpr float BAR_SPACING = 30.f;   // Vertical gap between stacked bars

    // ========== Colors ==========
    static const sf::Color COLOR_BAR_BACKGROUND;
    static const sf::Color COLOR_BAR_OUTLINE;
    static const sf::Color COLOR_HEALTH_FILL;
    static const sf::Color COLOR_TEXT;
};

#endif