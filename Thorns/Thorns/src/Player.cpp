#include "Player.h"

#define _USE_MATH_DEFINES
#include <math.h>

Player::Player()
    : m_speed(0.0f)
    , m_rotation(sf::degrees(0.0f))  // Initialize using sf::degrees()
{
    // Call initTexture() in the derived class constructor - this is safe
    initTexture();

    // Set player-specific properties after texture is loaded
    if (isValid())
    {
        setPosition(sf::Vector2f{ 150.f, 50.f });
    }
}

void Player::initTexture()
{
    // Player-specific texture loading
    loadTexture("ASSETS\\IMAGES\\PlayerShip.png", 64.f, 64.f);

    // Set origin to center of sprite for proper rotation
    if (isValid())
    {
        sf::FloatRect bounds = m_sprite.getLocalBounds();
        m_sprite.setOrigin(sf::Vector2f(bounds.size.x / 2.0f, bounds.size.y / 2.0f));
    }
}

void Player::update(sf::Time deltaTime)
{
    // Convert sf::Time to double milliseconds
    double dt = deltaTime.asMilliseconds();

    // Handle user input
    handleInput();

    // Handle speed and movement
    handleSpeed(dt);
}

// Method to handle user input for player movement and rotation
void Player::handleInput()
{
    using Key = sf::Keyboard::Key;

    if (sf::Keyboard::isKeyPressed(Key::W)) {
        increaseSpeed();
    }
    if (sf::Keyboard::isKeyPressed(Key::S)) {
        decreaseSpeed();
    }
    if (sf::Keyboard::isKeyPressed(Key::A)) {
        decreaseRotation();
    }
    if (sf::Keyboard::isKeyPressed(Key::D)) {
        increaseRotation();
    }
}

// Method to handle player speed based on input
void Player::handleSpeed(double dt)
{
    // Clamp speed within specified range
    m_speed = std::clamp(m_speed, MAX_REVERSE_SPEED, MAX_FORWARD_SPEED);

    // Forward vector adjusted so 0° means "up" instead of "right" (This is due to the current sprite being "UP" change depending on Project config)
    float angleRad = m_rotation.asRadians() - static_cast<float>(M_PI) / 2.0f;

    // Calculate new position
    sf::Vector2f pos = m_sprite.getPosition();
    pos.x += std::cos(angleRad) * m_speed * (dt / 1000.0);
    pos.y += std::sin(angleRad) * m_speed * (dt / 1000.0);

    // Update position and rotation
    m_sprite.setPosition(pos);
    m_sprite.setRotation(m_rotation);

    // Apply speed decay
    m_speed *= 0.99f;
}

// Method to increase player speed
void Player::increaseSpeed()
{
    m_speed += 5.0f;
}

// Method to decrease player speed
void Player::decreaseSpeed()
{
    m_speed -= 5.0f;
}

// Method to increase player rotation
void Player::increaseRotation()
{
    m_rotation += sf::degrees(1.0f);  // Add 1 degree using sf::degrees()
    m_rotation = m_rotation.wrapUnsigned();  // Wrap to 0-360 degrees automatically
}

// Method to decrease player rotation
void Player::decreaseRotation()
{
    m_rotation -= sf::degrees(1.0f);  // Subtract 1 degree using sf::degrees()
    m_rotation = m_rotation.wrapUnsigned();  // Wrap to 0-360 degrees automatically
}