#include "Enemy.h"

Enemy::Enemy()
    : m_speed(100.f)
{
}

bool Enemy::initialize(const std::string& texturePath)
{
    if (!m_sprite.loadTexture(texturePath, 64.f, 64.f))
        return false;

    // Set initial position
    m_sprite.setPosition(sf::Vector2f{ 300.f, 50.f });

    return true;
}

void Enemy::update(sf::Time deltaTime)
{
    // Move enemy down at constant speed
    sf::Vector2f movement{ 0.f, m_speed * deltaTime.asSeconds() };
    m_sprite.move(movement);

}

void Enemy::render(sf::RenderTarget& target) const
{
    m_sprite.render(target);
}

void Enemy::setPosition(const sf::Vector2f& pos)
{
    m_sprite.setPosition(pos);
}

sf::Vector2f Enemy::getPosition() const
{
    return m_sprite.getPosition();
}

sf::FloatRect Enemy::getBounds() const
{
    return m_sprite.getBounds();
}