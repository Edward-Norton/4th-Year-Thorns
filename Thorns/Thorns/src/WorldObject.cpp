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

bool WorldObject::isValid() const
{
    return m_sprite && m_sprite->isValid();
}