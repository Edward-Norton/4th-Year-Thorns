#ifndef PLAYER_HPP
#define PLAYER_HPP


#include "SpriteComponent.h"
#include <SFML/System/Time.hpp>

class InputController;

class Player
{
public:
    Player();
    ~Player() = default;

    // Initialization
    bool initialize(const std::string& texturePath);

    // Updatable interface
    void update(sf::Time deltaTime);

    // Renderable interface
    void render(sf::RenderTarget& target) const;

    // Handle input
    void handleInput(const InputController& input);

    // Position
    void setPosition(const sf::Vector2f& pos);
    sf::Vector2f getPosition() const;

    // Collision
    sf::FloatRect getBounds() const;

    // State
    bool isValid() const { return m_sprite.isValid(); }
    float getSpeed() const { return m_speed; }
    sf::Angle getRotation() const { return m_rotation; }

private:
    void handleSpeed(double deltaTime);
    void increaseSpeed();
    void decreaseSpeed();
    void increaseRotation();
    void decreaseRotation();

    // Components
    SpriteComponent m_sprite;

    // Player state
    float m_speed;
    sf::Angle m_rotation;

    static constexpr float MAX_FORWARD_SPEED = 200.0f;
    static constexpr float MAX_REVERSE_SPEED = -100.0f;
};

#endif