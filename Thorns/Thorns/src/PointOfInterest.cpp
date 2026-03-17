#include "PointOfInterest.h"
#include "CollisionManager.h"
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
    m_collisionShapes.push_back(defaultCollision);
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
    sf::Vector2f offset = pos - m_worldPosition;
    m_worldPosition = pos;

    if (m_sprite)
        m_sprite->setPosition(pos);

    // Translate all shapes by the same offset
    for (auto& shape : m_collisionShapes)
    {
        std::visit([&](auto& s)
            {
                using T = std::decay_t<decltype(s)>;
                if constexpr (std::is_same_v<T, sf::FloatRect>)
                {
                    s.position += offset;
                }
                else if constexpr (std::is_same_v<T, CollisionPolygon>)
                {
                    for (auto& pt : s.points)
                        pt += offset;
                }
            }, shape);
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
    for (const auto& rect : getCollisionRects())
    {
        if (rect.contains(worldPos))
            return true;
    }
    return false;
}

bool PointOfInterest::checkEntityCollision(const sf::FloatRect& entityBounds) const
{
    for (const auto& shape : m_collisionShapes)
    {
        bool hit = std::visit([&](const auto& s) -> bool
            {
                using T = std::decay_t<decltype(s)>;
                if constexpr (std::is_same_v<T, sf::FloatRect>)
                    return entityBounds.findIntersection(s).has_value();
                else if constexpr (std::is_same_v<T, CollisionPolygon>)
                    return CollisionManager::aabbVsPolygon(entityBounds, s.points);
                return false;
            }, shape);

        if (hit) return true;
    }
    return false;
}

void PointOfInterest::addCollisionShape(const CollisionShape& shape)
{
    m_collisionShapes.push_back(shape);
}

void PointOfInterest::clearCollisionShapes()
{
    m_collisionShapes.clear();
}

void PointOfInterest::addCollisionRect(const sf::FloatRect& rect)
{
    m_collisionShapes.push_back(rect);
}

void PointOfInterest::clearCollisionRects()
{
    m_collisionShapes.clear();
}

std::vector<sf::FloatRect> PointOfInterest::getCollisionRects() const
{
    std::vector<sf::FloatRect> rects;
    rects.reserve(m_collisionShapes.size());

    for (const auto& shape : m_collisionShapes)
    {
        std::visit([&](const auto& s)
            {
                using T = std::decay_t<decltype(s)>;
                if constexpr (std::is_same_v<T, sf::FloatRect>)
                {
                    rects.push_back(s);
                }
                else if constexpr (std::is_same_v<T, CollisionPolygon>)
                {
                    // AABB of polygon for tile marking
                    float minX = s.points[0].x, maxX = s.points[0].x;
                    float minY = s.points[0].y, maxY = s.points[0].y;
                    for (const auto& pt : s.points)
                    {
                        minX = std::min(minX, pt.x); maxX = std::max(maxX, pt.x);
                        minY = std::min(minY, pt.y); maxY = std::max(maxY, pt.y);
                    }
                    rects.emplace_back(
                        sf::Vector2f(minX, minY),
                        sf::Vector2f(maxX - minX, maxY - minY)
                    );
                }
            }, shape);
    }

    return rects;
}

bool PointOfInterest::hasSprite() const
{
    return m_sprite && m_sprite->isValid();
}