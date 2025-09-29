#include "Enemy.h"

Enemy::Enemy()
{
    // Call initTexture() in the derived class constructor
    initTexture();

    // Set enemy-specific properties after texture is loaded
    if (isValid())
    {
        m_speed = 100.f;
        setPosition(sf::Vector2f{ 300.f, 50.f });
    }
}

void Enemy::initTexture()
{
    // Enemy-specific texture loading
    loadTexture("ASSETS\\IMAGES\\EnemyShip.png", 64.f, 64.f);
}

void Enemy::update(sf::Time deltaTime)
{
    // Move enemy down at constant speed
    sf::Vector2f movement{ 0.f, m_speed * deltaTime.asSeconds() };
    m_sprite.move(movement);
}