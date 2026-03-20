#include "HUDComponent.h"
#include "AssetPaths.h"
#include <string>

// ========== Static color definitions ==========
const sf::Color HUDComponent::COLOR_BAR_BACKGROUND = sf::Color(30, 30, 30, 200);
const sf::Color HUDComponent::COLOR_BAR_OUTLINE = sf::Color(180, 180, 180, 200);
const sf::Color HUDComponent::COLOR_HEALTH_FILL = sf::Color(200, 50, 50);
const sf::Color HUDComponent::COLOR_TEXT = sf::Color(220, 220, 220);




bool HUDComponent::initialize(const std::string& fontPath)
{
    if (!m_font.openFromFile(fontPath))
        return false;

    m_fontLoaded = true;
    return true;
}

void HUDComponent::render(sf::RenderTarget& target) const
{
    if (!m_fontLoaded)
        return;

}


void HUDComponent::drawBar(sf::RenderTarget& target,
    float screenY,
    float ratio,
    const sf::Color& fillColor,
    const std::string& label,
    float current,
    float max) const
{
    const float clampedRatio = std::clamp(ratio, 0.f, 1.f);

    // Background (unfilled portion)
    sf::RectangleShape background(sf::Vector2f(BAR_WIDTH, BAR_HEIGHT));
    background.setPosition(sf::Vector2f(BAR_X, screenY));
    background.setFillColor(COLOR_BAR_BACKGROUND);
    background.setOutlineThickness(1.f);
    background.setOutlineColor(COLOR_BAR_OUTLINE);
    target.draw(background);

    // Filled portion
    if (clampedRatio > 0.f)
    {
        sf::RectangleShape fill(sf::Vector2f(BAR_WIDTH * clampedRatio, BAR_HEIGHT));
        fill.setPosition(sf::Vector2f(BAR_X, screenY));
        fill.setFillColor(fillColor);
        target.draw(fill);
    }

    // Label to the left of the bar ("HP")
    sf::Text labelText(m_font);
    labelText.setString(label);
    labelText.setCharacterSize(14);
    labelText.setFillColor(COLOR_TEXT);
    // Right-align the label so it sits flush against BAR_X
    float labelWidth = labelText.getLocalBounds().size.x;
    labelText.setPosition(sf::Vector2f(BAR_X - labelWidth + LABEL_OFFSET_X - 4.f, screenY + 1.f));
    target.draw(labelText);

    // Value text to the right of the bar ("75 / 100")
    std::string valueStr = std::to_string(static_cast<int>(current))
        + " / "
        + std::to_string(static_cast<int>(max));

    sf::Text valueText(m_font);
    valueText.setString(valueStr);
    valueText.setCharacterSize(13);
    valueText.setFillColor(COLOR_TEXT);
    valueText.setPosition(sf::Vector2f(BAR_X + BAR_WIDTH + 6.f, screenY + 2.f));
    target.draw(valueText);
}