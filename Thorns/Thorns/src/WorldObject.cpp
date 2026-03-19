#include "WorldObject.h"
#include <iostream>

WorldObject::WorldObject(Type type, const sf::Vector2f& worldPos)
    : m_type(type)
    , m_worldPosition(worldPos)
    , m_sprite(nullptr)
    , m_collisionShapes(nullptr)
    , m_shapeOffset(0.f, 0.f)
{
}

void WorldObject::render(sf::RenderTarget& target) const
{
    if (m_sprite && m_sprite->isValid())
    {
        m_sprite->render(target);
    }
}

void WorldObject::setPosition(const sf::Vector2f& pos)
{
    m_shapeOffset += (pos - m_worldPosition);
    m_worldPosition = pos;
    if (m_sprite)
    {
        m_sprite->setPosition(pos);
    }
}

sf::FloatRect WorldObject::getBounds() const
{
    if (m_sprite && m_sprite->isValid())
    {
        return m_sprite->getBounds();
    }

    // Fallback: small bounds at position
    return sf::FloatRect(sf::Vector2f(m_worldPosition.x - 16.f, m_worldPosition.y - 16.f),
        sf::Vector2f(32.f, 32.f));
}

bool WorldObject::loadSpriteFromTexture(const sf::Texture& sharedTexture, const sf::IntRect& textureRect, const sf::Vector2f& size)
{
    m_sprite = std::make_unique<SpriteComponent>();

    // Use the shared texture instead of loading from file
    if (!m_sprite->setSharedTexture(sharedTexture, size.x, size.y, textureRect))
    {
        std::cerr << "Failed to set shared texture for WorldObject\n";
        m_sprite.reset();
        return false;
    }

    // Center sprite origin for proper positioning
    m_sprite->centerOrigin();
    m_sprite->setPosition(m_worldPosition);

    return true;
}

void WorldObject::setCollisionShapes(const std::vector<CollisionShape>* shapes, const sf::Vector2f& templateOrigin)
{
    m_collisionShapes = shapes;
    m_shapeOffset = m_worldPosition - templateOrigin;
}

std::vector<CollisionShape> WorldObject::getWorldSpaceShapes() const
{
    if (!m_collisionShapes)
        return {};

    std::vector<CollisionShape> result;
    result.reserve(m_collisionShapes->size());

    for (const auto& shape : *m_collisionShapes)
    {
        // Translate each shape by m_shapeOffset into world space
        std::visit([&](const auto& s)
        {
                using T = std::decay_t<decltype(s)>;

                if constexpr (std::is_same_v<T, sf::FloatRect>)
                {
                    result.emplace_back(sf::FloatRect(
                        sf::Vector2f(s.position.x + m_shapeOffset.x,
                            s.position.y + m_shapeOffset.y),
                        s.size
                    ));
                }
                else if constexpr (std::is_same_v<T, CollisionPolygon>)
                {
                    CollisionPolygon worldPoly;
                    worldPoly.points.reserve(s.points.size());
                    for (const auto& pt : s.points)
                        worldPoly.points.emplace_back(pt.x + m_shapeOffset.x,
                            pt.y + m_shapeOffset.y);
                    result.emplace_back(std::move(worldPoly));
                }
         }, shape);
    }

    return result;
}

bool WorldObject::isValid() const
{
    return m_sprite && m_sprite->isValid();
}