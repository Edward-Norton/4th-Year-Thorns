#include "DayTimerComponent.h"
#include <iomanip>
#include <sstream>
#include <iostream>

DayTimerComponent::DayTimerComponent()
    : m_duration(60.f)
    , m_remaining(60.f)
    , m_expired(false)
    , m_fontLoaded(false)
{
}

bool DayTimerComponent::initialize(float duration, const std::string& fontPath)
{
    m_duration = duration;
    m_remaining = duration;
    m_expired = false;

    if (!m_font.openFromFile(fontPath))
    {
        std::cerr << "DayTimerComponent: Failed to load font: " << fontPath << "\n";
        return false;
    }

    m_fontLoaded = true;
    return true;
}

void DayTimerComponent::update(sf::Time deltaTime)
{
    
    if (m_expired)
        return;

    m_remaining -= deltaTime.asSeconds();

    if (m_remaining <= 0.f)
    {
        m_remaining = 0.f;
        m_expired = true;

        if (m_onExpired)
            m_onExpired();
    }
}

void DayTimerComponent::render(sf::RenderTarget& target) const
{
    if (!m_fontLoaded)
        return;

    const float screenW = static_cast<float>(target.getSize().x);
    const float panelX = screenW - PANEL_WIDTH - PANEL_RIGHT_MARGIN;
    const float panelY = PANEL_TOP_MARGIN;

    
    sf::RectangleShape panel(sf::Vector2f(PANEL_WIDTH, PANEL_HEIGHT));
    panel.setPosition(sf::Vector2f(panelX, panelY));
    panel.setFillColor(sf::Color(30, 30, 30, 200));
    panel.setOutlineThickness(1.f);
    panel.setOutlineColor(sf::Color(180, 180, 180, 200));
    target.draw(panel);

    
    sf::Text label(m_font);
    label.setString("DAY ENDS IN");
    label.setCharacterSize(FONT_SIZE_LABEL);
    label.setFillColor(sf::Color(200, 200, 200));

    sf::FloatRect lb = label.getLocalBounds();
    label.setPosition(sf::Vector2f(
        panelX + (PANEL_WIDTH - lb.size.x) / 2.f,
        panelY + 6.f
    ));
    target.draw(label);

    sf::Text timeText(m_font);
    timeText.setString(formatTime());
    timeText.setCharacterSize(FONT_SIZE_TIME);

    const sf::Color timeColor = (m_remaining <= 10.f)
        ? sf::Color(220, 60, 60)
        : sf::Color(240, 240, 240);

    timeText.setFillColor(timeColor);

    sf::FloatRect tb = timeText.getLocalBounds();
    timeText.setPosition(sf::Vector2f(
        panelX + (PANEL_WIDTH - tb.size.x) / 2.f - tb.position.x,
        panelY + PANEL_HEIGHT - tb.size.y - 8.f - tb.position.y
    ));
    target.draw(timeText);
}

void DayTimerComponent::reset()
{
    m_remaining = m_duration;
    m_expired = false;
}

std::string DayTimerComponent::formatTime() const
{
    int total = static_cast<int>(std::ceil(m_remaining));
    int minutes = total / 60;
    int seconds = total % 60;

    std::ostringstream oss;
    oss << minutes << ":" << std::setfill('0') << std::setw(2) << seconds;
    return oss.str();
}