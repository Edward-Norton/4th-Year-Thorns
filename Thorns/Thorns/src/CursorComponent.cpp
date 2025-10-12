#include "CursorComponent.h"

CursorComponent::CursorComponent()
    : m_position(0.f, 0.f)
    , m_visible(true)
    , m_radius(5.f)
{
}

bool CursorComponent::initialize(float radius)
{
    m_radius = radius;
    m_cursorShape.setRadius(m_radius);
    m_cursorShape.setOrigin(sf::Vector2f(m_radius, m_radius)); // Center the cursor
    m_cursorShape.setFillColor(sf::Color::Transparent);
    m_cursorShape.setOutlineThickness(2.f);
    m_cursorShape.setOutlineColor(sf::Color::White);

    return true;
}

void CursorComponent::update(const sf::Vector2f& mousePosition)
{
    m_position = mousePosition;
    m_cursorShape.setPosition(m_position);
}

void CursorComponent::render(sf::RenderTarget& target) const
{
    if (!m_visible)
        return;

    target.draw(m_cursorShape);
}

void CursorComponent::setColor(const sf::Color& color)
{
    m_cursorShape.setOutlineColor(color);
}

void CursorComponent::setRadius(float radius)
{
    m_radius = radius;
    m_cursorShape.setRadius(m_radius);
    m_cursorShape.setOrigin(sf::Vector2f(m_radius, m_radius));
}