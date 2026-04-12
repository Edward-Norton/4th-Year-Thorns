#include "HUDComponent.h"
#include "AssetPaths.h"
#include <string>

const sf::Color HUDComponent::COLOR_BAR_BACKGROUND = sf::Color(30, 30, 30, 200);
const sf::Color HUDComponent::COLOR_BAR_OUTLINE = sf::Color(180, 180, 180, 200);
const sf::Color HUDComponent::COLOR_HEALTH_FILL = sf::Color(200, 50, 50);       
const sf::Color HUDComponent::COLOR_STAMINA_FILL = sf::Color(220, 220, 220);     
const sf::Color HUDComponent::COLOR_HUNGER_FILL = sf::Color(210, 120, 30);      
const sf::Color HUDComponent::COLOR_WATER_FILL = sf::Color(50, 120, 220);      
const sf::Color HUDComponent::COLOR_TEXT = sf::Color(220, 220, 220);

HUDComponent::HUDComponent(const HealthComponent& health, 
    const StatComponent& stamina,
    const StatComponent& hunger,
    const StatComponent& water)
    : m_health(health)
    , m_stamina(stamina)
    , m_hunger(hunger)
    , m_water(water)
    , m_fontLoaded(false)
{
}

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

    
    drawBar(target,
        BAR_Y,
        m_health.getHealthRatio(),
        COLOR_HEALTH_FILL,
        "HP",
        m_health.getCurrentHealth(),
        m_health.getMaxHealth());

    
    drawBar(target, 
        BAR_Y + BAR_SPACING * 1,
        m_stamina.getRatio(), 
        COLOR_STAMINA_FILL,
        "ST", m_stamina.getValue(), 
        m_stamina.getMaxValue());

    
    drawBar(target, 
        BAR_Y + BAR_SPACING * 2,
        m_hunger.getRatio(), 
        COLOR_HUNGER_FILL,
        "HG", m_hunger.getValue(), 
        m_hunger.getMaxValue());

    
    drawBar(target, 
        BAR_Y + BAR_SPACING * 3,
        m_water.getRatio(), 
        COLOR_WATER_FILL,
        "WA", m_water.getValue(), 
        m_water.getMaxValue());
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

    
    sf::RectangleShape background(sf::Vector2f(BAR_WIDTH, BAR_HEIGHT));
    background.setPosition(sf::Vector2f(BAR_X, screenY));
    background.setFillColor(COLOR_BAR_BACKGROUND);
    background.setOutlineThickness(1.f);
    background.setOutlineColor(COLOR_BAR_OUTLINE);
    target.draw(background);

    
    if (clampedRatio > 0.f)
    {
        sf::RectangleShape fill(sf::Vector2f(BAR_WIDTH * clampedRatio, BAR_HEIGHT));
        fill.setPosition(sf::Vector2f(BAR_X, screenY));
        fill.setFillColor(fillColor);
        target.draw(fill);
    }

    
    sf::Text labelText(m_font);
    labelText.setString(label);
    labelText.setCharacterSize(14);
    labelText.setFillColor(COLOR_TEXT);
    
    float labelWidth = labelText.getLocalBounds().size.x;
    labelText.setPosition(sf::Vector2f(BAR_X - labelWidth + LABEL_OFFSET_X - 4.f, screenY + 1.f));
    target.draw(labelText);

    
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