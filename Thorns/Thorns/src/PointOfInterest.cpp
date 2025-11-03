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