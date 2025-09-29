#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "Entity.h"

class Player : public Entity
{
public:
    Player();
    ~Player() = default;

    // Override the pure virtual functions
    void initTexture() override;
    void update(sf::Time deltaTime) override;

    // Movement and rotation methods
    void handleInput();
    void handleSpeed(double deltaTime);
    void increaseSpeed();
    void decreaseSpeed();
    void increaseRotation();
    void decreaseRotation();
private:
    float m_speed;                  // Current speed
    sf::Angle m_rotation;           // Current rotation using sf::Angle
    static constexpr const float MAX_FORWARD_SPEED = 200.0f;
    static constexpr const float MAX_REVERSE_SPEED = -100.0f;

};

#endif