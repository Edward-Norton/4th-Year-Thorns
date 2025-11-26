#include "PointOfInterest.h"
#include <iostream>

PointOfInterest::PointOfInterest(const std::string& name, Type type,
    const sf::Vector2f& worldPos, const sf::Vector2f& size)
    : m_name(name)
    , m_type(type)
    , m_worldPosition(worldPos)
    , m_size(size)
    , m_blocking(true)
    , m_sprite(nullptr)
{
    // Exclusion radius is based on POI size
    // Use diagonal length plus padding to ensure Voronoi sites don't spawn too close
    float diagonal = std::sqrt(size.x * size.x + size.y * size.y);
    m_exclusionRadius = diagonal * 0.6f; // 60% of diagonal as buffer

    // By default, create one collision rect that matches the full sprite bounds
    sf::FloatRect defaultCollision(
        sf::Vector2f(worldPos.x - size.x / 2.f,
            worldPos.y - size.y / 2.f),
        sf::Vector2f(size.x, size.y)
    );
    m_collisionRects.push_back(defaultCollision);
}

bool PointOfInterest::loadSprite(const std::string& spritePath)
{
    m_sprite = std::make_unique<SpriteComponent>();

    if (!m_sprite->loadTexture(spritePath, m_size.x, m_size.y))
    {
        std::cerr << "Failed to load POI sprite: " << spritePath << " for " << m_name << "\n";
        m_sprite.reset();
        return false;
    }

    // Center the sprite's origin
    m_sprite->centerOrigin();
    m_sprite->setPosition(m_worldPosition);

    std::cout << "Loaded POI sprite: " << m_name << " (" << m_size.x << "x" << m_size.y << ")\n";
    return true;
}

void PointOfInterest::render(sf::RenderTarget& target) const
{
    if (m_sprite && m_sprite->isValid())
    {
        m_sprite->render(target);
    }
}

void PointOfInterest::setPosition(const sf::Vector2f& pos)
{
    // Calculate offset from old position
    sf::Vector2f offset = pos - m_worldPosition;

    m_worldPosition = pos;

    if (m_sprite)
    {
        m_sprite->setPosition(pos);
    }

    // Update all collision rects to new position
    for (auto& rect : m_collisionRects)
    {
        rect.position += offset;
    }
}

sf::FloatRect PointOfInterest::getBounds() const
{
    // Primary bounds for ICollidable interface
    if (m_sprite && m_sprite->isValid())
    {
        return m_sprite->getBounds();
    }

    // Fallback: calculate from size
    return getVisualBounds();
}

sf::FloatRect PointOfInterest::getVisualBounds() const
{
    // Calculate AABB from center position and size
    float left = m_worldPosition.x - (m_size.x / 2.f);
    float top = m_worldPosition.y - (m_size.y / 2.f);

    return sf::FloatRect(sf::Vector2f(left, top), m_size);
}

bool PointOfInterest::contains(const sf::Vector2f& worldPos) const
{
    // Check against all collision rectangles
    for (const auto& rect : m_collisionRects)
    {
        if (rect.contains(worldPos))
            return true;
    }
    return false;
}

bool PointOfInterest::checkEntityCollision(const sf::FloatRect& entityBounds) const
{
    // Check collision with all rectangles
    for (const auto& rect : m_collisionRects)
    {
        if (entityBounds.findIntersection(rect).has_value())
            return true;
    }
    return false;
}

void PointOfInterest::addCollisionRect(const sf::FloatRect& rect)
{
    m_collisionRects.push_back(rect);
}

void PointOfInterest::clearCollisionRects()
{
    m_collisionRects.clear();
}

bool PointOfInterest::hasSprite() const
{
    return m_sprite && m_sprite->isValid();
}