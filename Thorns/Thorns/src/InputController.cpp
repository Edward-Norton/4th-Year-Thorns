#include "InputController.h"
#include <cmath>
#include <cstring>  // For memset and memcpy

InputController::InputController()
    : m_mousePosition(0.f, 0.f)
    , m_mousePressed(false)
    , m_previousMousePressed(false)
    , m_mouseClicked(false)
    , m_mouseReleased(false)
{
    initializeDefaultBindings();
}

void InputController::initializeDefaultBindings()
{
    // Gameplay
    m_keyBindings[InputAction::MoveUp] = sf::Keyboard::Key::W;
    m_keyBindings[InputAction::MoveDown] = sf::Keyboard::Key::S;
    m_keyBindings[InputAction::MoveLeft] = sf::Keyboard::Key::A;
    m_keyBindings[InputAction::MoveRight] = sf::Keyboard::Key::D;

    // Menu navigation
    m_keyBindings[InputAction::Pause] = sf::Keyboard::Key::P;
    m_keyBindings[InputAction::Menu] = sf::Keyboard::Key::Escape;
    m_keyBindings[InputAction::Confirm] = sf::Keyboard::Key::Enter;
    m_keyBindings[InputAction::Cancel] = sf::Keyboard::Key::Escape;
}

void InputController::update(const sf::Window& window)
{
    /*
     * Two-phase update:
     * 1. Update keyboard/gamepad actions
     * 2. Update mouse state
     */

     // Save previous states
    std::memcpy(m_previousState, m_currentState, sizeof(m_currentState));
    m_previousMousePressed = m_mousePressed;

    // Update current states
    updateKeyboardGamepad();
    updateMouse(window);

    // Calculate edge transitions for mouse
    m_mouseClicked = m_mousePressed && !m_previousMousePressed;
    m_mouseReleased = !m_mousePressed && m_previousMousePressed;
}

void InputController::updateKeyboardGamepad()
{
    // Clear action states
    std::memset(m_currentState, 0, sizeof(m_currentState));

    // Check keyboard
    for (const auto& [action, key] : m_keyBindings)
    {
        if (sf::Keyboard::isKeyPressed(key))
            m_currentState[static_cast<int>(action)] = true;
    }

    // Check gamepad (if connected)
    if (sf::Joystick::isConnected(m_activeGamepad))
    {
        // Analog stick to digital input
        float x = sf::Joystick::getAxisPosition(m_activeGamepad, sf::Joystick::Axis::X) / 100.0f;
        float y = sf::Joystick::getAxisPosition(m_activeGamepad, sf::Joystick::Axis::Y) / 100.0f;

        x = applyDeadzone(x);
        y = applyDeadzone(y);

        if (x < -0.5f) m_currentState[static_cast<int>(InputAction::MoveLeft)] = true;
        if (x > 0.5f) m_currentState[static_cast<int>(InputAction::MoveRight)] = true;
        if (y < -0.5f) m_currentState[static_cast<int>(InputAction::MoveUp)] = true;
        if (y > 0.5f) m_currentState[static_cast<int>(InputAction::MoveDown)] = true;

        // Gamepad buttons
        if (sf::Joystick::isButtonPressed(m_activeGamepad, 0)) // A button
            m_currentState[static_cast<int>(InputAction::Confirm)] = true;
        if (sf::Joystick::isButtonPressed(m_activeGamepad, 1)) // B button
            m_currentState[static_cast<int>(InputAction::Cancel)] = true;
        if (sf::Joystick::isButtonPressed(m_activeGamepad, 7)) // Start button
            m_currentState[static_cast<int>(InputAction::Pause)] = true;
    }
}


void InputController::updateMouse(const sf::Window& window)
{
    // Get mouse position relative to window
    sf::Vector2i pixelPos = sf::Mouse::getPosition(window);
    m_mousePosition = sf::Vector2f(static_cast<float>(pixelPos.x),
        static_cast<float>(pixelPos.y));

    // Check left mouse button
    m_mousePressed = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
}

float InputController::getHorizontalAxis() const
{
    // Prefer analog stick if gamepad is connected
    if (sf::Joystick::isConnected(m_activeGamepad))
    {
        float x = sf::Joystick::getAxisPosition(m_activeGamepad, sf::Joystick::Axis::X) / 100.0f;
        return applyDeadzone(x);
    }

    // Fall back to keyboard (digital input: -1, 0, or +1)
    float value = 0.0f;
    if (isPressed(InputAction::MoveLeft)) value -= 1.0f;
    if (isPressed(InputAction::MoveRight)) value += 1.0f;
    return value;
}

float InputController::getVerticalAxis() const
{
    // Prefer analog stick if gamepad is connected
    if (sf::Joystick::isConnected(m_activeGamepad))
    {
        float y = sf::Joystick::getAxisPosition(m_activeGamepad, sf::Joystick::Axis::Y) / 100.0f;
        return applyDeadzone(y);
    }

    // Fall back to keyboard (digital input: -1, 0, or +1)
    float value = 0.0f;
    if (isPressed(InputAction::MoveUp)) value -= 1.0f;
    if (isPressed(InputAction::MoveDown)) value += 1.0f;
    return value;
}

float InputController::applyDeadzone(float value) const
{
    if (std::abs(value) < m_deadzone)
        return 0.0f;

    float sign = (value > 0) ? 1.0f : -1.0f;
    return sign * ((std::abs(value) - m_deadzone) / (1.0f - m_deadzone));
}

// Allow runtime key remapping
void InputController::bindKey(InputAction action, sf::Keyboard::Key key)
{
    m_keyBindings[action] = key;
}

// Return currently bound key, or Unknown if not found
sf::Keyboard::Key InputController::getKeyBinding(InputAction action) const
{
    auto it = m_keyBindings.find(action);
    return it != m_keyBindings.end() ? it->second : sf::Keyboard::Key::Unknown;
}