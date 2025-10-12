#include "Player.h"
#include "InputController.h"
#include "MathUtilities.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <iostream>

Player::Player()
    : m_currentState(PlayerState::Idle)
    , m_active(true)
    , m_inputController(nullptr)
    , m_mousePosition(0.f, 0.f)
    , m_velocity(0.f, 0.f)
    , m_targetRotation(sf::degrees(0.f))
    , m_currentRotation(sf::degrees(0.f))
{
}

bool Player::initialize(const std::string& texturePath)
{
    // Initialize sprite from texture atlas
    if (!m_sprite.loadTexture(texturePath, 34.f, 50.f,
        sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(34, 50))))
        return false;

    m_sprite.centerOrigin();
    m_sprite.setPosition(sf::Vector2f{ 960.f, 540.f });  // Center of 1920x1080

    // Initialize cursor
    if (!m_cursor.initialize(8.f))
        return false;

    m_cursor.setColor(sf::Color::Yellow);

    return true;
}

void Player::update(sf::Time deltaTime)
{
    if (!m_active) return;

    // Update state machine based on current input
    updateState();

    // Update cursor visuals
    updateCursor();

    // Update rotation to face mouse
    updateRotation();

    // Update movement physics
    updateMovement(deltaTime);
}

void Player::updateWithInput(sf::Time deltaTime, const InputController& input, const sf::Vector2f& mousePosition)
{
    if (!m_active) return;

    // Cache input references
    m_inputController = &input;
    m_mousePosition = mousePosition;

    // Delegate to standard update
    update(deltaTime);
}

void Player::render(sf::RenderTarget& target) const
{
    if (!m_active) return;

    m_sprite.render(target);
    m_cursor.render(target);
}

void Player::updateState()
{
    if (!m_inputController) return;

    // Get raw movement input
    sf::Vector2f moveInput = calculateMovementInput();
    bool isMoving = (moveInput.x != 0.f || moveInput.y != 0.f);
    bool isSprinting = m_inputController->isPressed(InputAction::Sprint);

    // Determine new state
    PlayerState newState = m_currentState;

    if (!isMoving)
    {
        newState = PlayerState::Idle;
    }
    else if (isSprinting)
    {
        newState = PlayerState::Sprint;
    }
    else
    {
        newState = PlayerState::Walk;
    }

    // Change state if different
    if (newState != m_currentState)
    {
        changeState(newState);
    }
}

void Player::changeState(PlayerState newState)
{
    // Exit current state (optional cleanup)
    switch (m_currentState)
    {
    case PlayerState::Idle:
        break;
    case PlayerState::Walk:
        break;
    case PlayerState::Sprint:
        break;
    }

    // Enter new state
    m_currentState = newState;

    // Debug output
    switch (m_currentState)
    {
    case PlayerState::Idle:
        std::cout << "State: IDLE\n";
        break;
    case PlayerState::Walk:
        std::cout << "State: WALK\n";
        break;
    case PlayerState::Sprint:
        std::cout << "State: SPRINT\n";
        break;
    }
}

void Player::updateMovement(sf::Time deltaTime)
{
    if (!m_inputController) return;

    float dt = deltaTime.asSeconds();

    // Get movement input direction
    sf::Vector2f moveInput = calculateMovementInput();
    float inputMagnitude = MathUtils::magnitude(moveInput);

    if (inputMagnitude > 0.f)
    {
        // Moving - set velocity based on state
        sf::Vector2f direction = MathUtils::normalize(moveInput);
        float targetSpeed = getCurrentSpeed();

        // Instant velocity change
        m_velocity = direction * targetSpeed;
    }
    else
    {
        // Not moving - apply deceleration
        float currentSpeed = MathUtils::magnitude(m_velocity);

        if (currentSpeed > 0.f)
        {
            sf::Vector2f direction = MathUtils::normalize(m_velocity);
            float decel = DECELERATION * dt;
            currentSpeed = std::max(0.f, currentSpeed - decel);
            m_velocity = direction * currentSpeed;
        }
    }

    // Apply velocity to position
    sf::Vector2f movement = m_velocity * dt;
    m_sprite.move(movement);
}

void Player::updateRotation()
{
    // Calculate direction from player to mouse
    sf::Vector2f playerPos = m_sprite.getPosition();
    sf::Vector2f direction = m_mousePosition - playerPos;

    // Calculate angle in degrees
    float angleDeg = MathUtils::vectorToAngleDegrees(direction);

    // Set target rotation
    m_targetRotation = sf::degrees(angleDeg);

    // For Darkwood-style, use instant rotation
    // (You can add smooth interpolation here if desired)
    m_currentRotation = m_targetRotation;
    m_sprite.setRotation(m_currentRotation);
}

void Player::updateCursor()
{
    m_cursor.update(m_mousePosition);
}

sf::Vector2f Player::calculateMovementInput() const
{
    if (!m_inputController) return sf::Vector2f(0.f, 0.f);

    sf::Vector2f input(0.f, 0.f);

    if (m_inputController->isPressed(InputAction::MoveUp))    input.y -= 1.f;
    if (m_inputController->isPressed(InputAction::MoveDown))  input.y += 1.f;
    if (m_inputController->isPressed(InputAction::MoveLeft))  input.x -= 1.f;
    if (m_inputController->isPressed(InputAction::MoveRight)) input.x += 1.f;

    return input;
}

float Player::getCurrentSpeed() const
{
    switch (m_currentState)
    {
    case PlayerState::Idle:
        return 0.f;
    case PlayerState::Walk:
        return WALK_SPEED;
    case PlayerState::Sprint:
        return SPRINT_SPEED;
    default:
        return 0.f;
    }
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