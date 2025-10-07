#include "SpriteComponent.h"
#include <iostream>

SpriteComponent::SpriteComponent()
    : m_texture()
    , m_sprite(m_texture)
    , m_isValid(false)
{
}

bool SpriteComponent::loadTexture(const std::string& texturePath, float width, float height)
{
    if (!m_texture.loadFromFile(texturePath))
    {
        std::cerr << "SpriteComponent::loadTexture() - failed to load texture: " << texturePath << '\n';
        m_isValid = false;
        return false;
    }

    m_sprite.setTexture(m_texture, true);
    setSize(width, height);
    m_isValid = true;
    return true;
}

void SpriteComponent::render(sf::RenderTarget& target) const
{
    if (m_isValid)
    {
        target.draw(m_sprite);
    }
}

void SpriteComponent::setPosition(const sf::Vector2f& pos)
{
    m_sprite.setPosition(pos);
}

sf::Vector2f SpriteComponent::getPosition() const
{
    return m_sprite.getPosition();
}

void SpriteComponent::setRotation(sf::Angle angle)
{
    m_sprite.setRotation(angle);
}

sf::Angle SpriteComponent::getRotation() const
{
    return m_sprite.getRotation();
}

void SpriteComponent::setOrigin(const sf::Vector2f& origin)
{
    m_sprite.setOrigin(origin);
}

void SpriteComponent::centerOrigin()
{
    sf::FloatRect bounds = m_sprite.getLocalBounds();
    m_sprite.setOrigin(sf::Vector2f(bounds.size.x / 2.0f, bounds.size.y / 2.0f));
}

void SpriteComponent::setSize(float width, float height)
{
    sf::Vector2u texSize = m_texture.getSize();
    if (texSize.x == 0 || texSize.y == 0) return;

    float scaleX = width / static_cast<float>(texSize.x);
    float scaleY = height / static_cast<float>(texSize.y);
    m_sprite.setScale(sf::Vector2f{ scaleX, scaleY });
}

sf::Vector2f SpriteComponent::getSize() const
{
    sf::Vector2f scale = m_sprite.getScale();
    sf::Vector2u texSize = m_texture.getSize();
    return sf::Vector2f{ texSize.x * scale.x, texSize.y * scale.y };
}

void SpriteComponent::setScale(const sf::Vector2f& scale)
{
    m_sprite.setScale(scale);
}

void SpriteComponent::move(const sf::Vector2f& offset)
{
    m_sprite.move(offset);
}

sf::FloatRect SpriteComponent::getBounds() const
{
    return m_sprite.getGlobalBounds();
}

sf::FloatRect SpriteComponent::getLocalBounds() const
{
    return m_sprite.getLocalBounds();
}