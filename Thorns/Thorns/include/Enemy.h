#ifndef ENEMY_HPP
#define ENEMY_HPP

#include "SpriteComponent.h"
#include <SFML/System/Time.hpp>

class Enemy
{
public:
    Enemy();
    ~Enemy() = default;

    // Initialization
    bool initialize(const std::string& texturePath);

    // IUpdatable interface
    void update(sf::Time deltaTime);

    // IRenderable interface
    void render(sf::RenderTarget& target) const;

    // Position
    void setPosition(const sf::Vector2f& pos);
    sf::Vector2f getPosition() const;

    // Movement
    void setSpeed(float speed) { m_speed = speed; }
    float getSpeed() const { return m_speed; }

    // Collision
    sf::FloatRect getBounds() const;

    // State
    bool isValid() const { return m_sprite.isValid(); }

private:
    // Components
    SpriteComponent m_sprite;

    // Enemy state
    float m_speed;
};

#endif