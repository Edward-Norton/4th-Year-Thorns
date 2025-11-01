#include "PointOfInterest.h"

PointOfInterest::PointOfInterest(const std::string& name, Type type,
    const sf::Vector2f& worldPos, const sf::Vector2f& size)
    : m_name(name)
    , m_type(type)
    , m_worldPosition(worldPos)
    , m_size(size)
    , m_blocking(true)
{
    // Exclusion radius is based on POI size
    // Use diagonal length plus padding to ensure Voronoi sites don't spawn too close
    float diagonal = std::sqrt(size.x * size.x + size.y * size.y);
    m_exclusionRadius = diagonal * 0.6f; // 60% of diagonal as buffer
}

sf::FloatRect PointOfInterest::getBounds() const
{
    // Calculate AABB from center position and size
    float left = m_worldPosition.x - (m_size.x / 2.f);
    float top = m_worldPosition.y - (m_size.y / 2.f);

    return sf::FloatRect(sf::Vector2f(left, top), m_size);
}

bool PointOfInterest::contains(const sf::Vector2f& worldPos) const
{
    return getBounds().contains(worldPos);
}

void PointOfInterest::renderDebug(sf::RenderTarget& target, const sf::Color& color) const
{
    // Draw bounding box
    sf::RectangleShape bounds;
    bounds.setPosition(sf::Vector2f(getBounds().position));
    bounds.setSize(sf::Vector2f(getBounds().size));
    bounds.setFillColor(sf::Color(color.r, color.g, color.b, 50));
    bounds.setOutlineColor(color);
    bounds.setOutlineThickness(2.f);
    target.draw(bounds);

    // Draw exclusion radius circle
    sf::CircleShape exclusion(m_exclusionRadius);
    exclusion.setOrigin(sf::Vector2f(m_exclusionRadius, m_exclusionRadius));
    exclusion.setPosition(m_worldPosition);
    exclusion.setFillColor(sf::Color(color.r, color.g, color.b, 20));
    exclusion.setOutlineColor(sf::Color(color.r, color.g, color.b, 100));
    exclusion.setOutlineThickness(1.f);
    target.draw(exclusion);

    // Draw center cross
    sf::RectangleShape crossH(sf::Vector2f(20.f, 2.f));
    crossH.setOrigin(sf::Vector2f(10.f, 1.f));
    crossH.setPosition(m_worldPosition);
    crossH.setFillColor(color);
    target.draw(crossH);

    sf::RectangleShape crossV(sf::Vector2f(2.f, 20.f));
    crossV.setOrigin(sf::Vector2f(1.f, 10.f));
    crossV.setPosition(m_worldPosition);
    crossV.setFillColor(color);
    target.draw(crossV);
}