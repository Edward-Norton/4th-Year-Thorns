#include "Player.h"
#include "InputController.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <algorithm>

Player::Player()
    : m_speed(0.0f)
    , m_rotation(sf::degrees(0.0f))
{
}

bool Player::initialize(const std::string& texturePath)
{
    if (!m_sprite.loadTexture(texturePath, 64.f, 64.f))
        return false;

    // Center the origin for proper rotation
    m_sprite.centerOrigin();

    // Set initial position
    m_sprite.setPosition(sf::Vector2f{ 150.f, 50.f });

    return true;
}

void Player::update(sf::Time deltaTime)
{
    // Convert sf::Time to double milliseconds
    double dt = deltaTime.asMilliseconds();

    // Handle speed and movement
    handleSpeed(dt);
}

void Player::render(sf::RenderTarget& target) const
{
    m_sprite.render(target);
}

void Player::handleInput(const InputController& input)
{
    if (input.isPressed(InputAction::MoveUp))
        increaseSpeed();

    if (input.isPressed(InputAction::MoveDown))
        decreaseSpeed();

    if (input.isPressed(InputAction::MoveLeft))
        decreaseRotation();

    if (input.isPressed(InputAction::MoveRight))
        increaseRotation();
}

void Player::handleSpeed(double dt)
{
    // Clamp speed within specified range
    m_speed = std::clamp(m_speed, MAX_REVERSE_SPEED, MAX_FORWARD_SPEED);

    // Forward vector adjusted so 0° means "up" instead of "right"
    float angleRad = m_rotation.asRadians() - static_cast<float>(M_PI) / 2.0f;

    // Calculate movement vector
    sf::Vector2f movement;
    movement.x = std::cos(angleRad) * m_speed * (dt / 1000.0f);
    movement.y = std::sin(angleRad) * m_speed * (dt / 1000.0f);

    // Update position and rotation
    m_sprite.move(movement);
    m_sprite.setRotation(m_rotation);

    // Apply speed decay
    m_speed *= 0.99f;
}

void Player::increaseSpeed()
{
    m_speed += 5.0f;
}

void Player::decreaseSpeed()
{
    m_speed -= 5.0f;
}

void Player::increaseRotation()
{
    m_rotation += sf::degrees(1.0f);
    m_rotation = m_rotation.wrapUnsigned();
}

void Player::decreaseRotation()
{
    m_rotation -= sf::degrees(1.0f);
    m_rotation = m_rotation.wrapUnsigned();
}

void Player::setPosition(const sf::Vector2f& pos)
{
    m_sprite.setPosition(pos);
}

sf::Vector2f Player::getPosition() const
{
    return m_sprite.getPosition();
}

sf::FloatRect Player::getBounds() const
{
    return m_sprite.getBounds();
}