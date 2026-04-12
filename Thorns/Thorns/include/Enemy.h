#ifndef ENEMY_HPP
#define ENEMY_HPP

#include "SpriteComponent.h"
#include <SFML/System/Time.hpp>

class Enemy
{
public:
    Enemy();
    ~Enemy() = default;

    
    bool initialize(const std::string& texturePath);

    
    void update(sf::Time deltaTime);

    
    void render(sf::RenderTarget& target) const;

    
    void setPosition(const sf::Vector2f& pos);
    sf::Vector2f getPosition() const;

    
    void setSpeed(float speed) { m_speed = speed; }
    float getSpeed() const { return m_speed; }

    
    sf::FloatRect getBounds() const;

    
    bool isValid() const { return m_sprite.isValid(); }

private:
    
    SpriteComponent m_sprite;

    
    float m_speed;
};

#endif