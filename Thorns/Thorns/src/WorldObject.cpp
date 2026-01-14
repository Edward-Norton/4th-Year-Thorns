#include "WorldObject.h"
#include <iostream>

WorldObject::WorldObject(Type type, const sf::Vector2f& worldPos)
    : m_type(type)
    , m_worldPosition(worldPos)
    , m_sprite(nullptr)
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
    m_worldPosition = pos;
    if (m_sprite)
    {
        m_sprite->setPosition(pos);
    }
}

bool WorldObject::loadSprite(const std::string& atlasPath, const sf::IntRect& textureRect, const sf::Vector2f& size)
{
    m_sprite = std::make_unique<SpriteComponent>();

    if (!m_sprite->loadTexture(atlasPath, size.x, size.y, textureRect))
    {
        std::cerr << "Failed to load WorldObject sprite from atlas\n";
        m_sprite.reset();
        return false;
    }

    // Center sprite origin for proper positioning
    m_sprite->centerOrigin();
    m_sprite->setPosition(m_worldPosition);

    return true;
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

bool WorldObject::isValid() const
{
    return m_sprite && m_sprite->isValid();
}