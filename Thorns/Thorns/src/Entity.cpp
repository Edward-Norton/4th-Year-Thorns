#include "Entity.h"
#include <iostream>

Entity::Entity()
    : m_texture()
    , m_sprite(m_texture)
    , m_isValid(false)
{
    // Note: DON'T call initTexture() here - derived classes will call it in their constructors
}

bool Entity::loadTexture(const std::string& texturePath, float width, float height)
{
    if (!m_texture.loadFromFile(texturePath))
    {
        std::cerr << "Entity::loadTexture() - failed to load texture: " << texturePath << '\n';
        m_isValid = false;
        return false;
    }

    m_sprite.setTexture(m_texture, true);
    setSize(width, height);
    m_isValid = true;
    return true;
}

void Entity::render(sf::RenderTarget& target) const
{
    if (m_isValid)
    {
        target.draw(m_sprite);
    }
}

void Entity::setPosition(const sf::Vector2f& pos)
{
    m_sprite.setPosition(pos);
}

sf::Vector2f Entity::getPosition() const
{
    return m_sprite.getPosition();
}

void Entity::setSpeed(float speed)
{
    m_speed = speed;
}

float Entity::getSpeed() const
{
    return m_speed;
}

void Entity::setSize(float width, float height)
{
    sf::Vector2u texSize = m_texture.getSize();
    if (texSize.x == 0 || texSize.y == 0) return;

    float scaleX = width / static_cast<float>(texSize.x);
    float scaleY = height / static_cast<float>(texSize.y);
    m_sprite.setScale(sf::Vector2f{ scaleX, scaleY });
}

sf::Vector2f Entity::getSize() const
{
    sf::Vector2f scale = m_sprite.getScale();
    sf::Vector2u texSize = m_texture.getSize();
    return sf::Vector2f{ texSize.x * scale.x, texSize.y * scale.y };
}

sf::FloatRect Entity::getBounds() const
{
    return m_sprite.getGlobalBounds();
}