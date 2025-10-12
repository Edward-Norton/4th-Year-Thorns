#include "InputController.h"
#include <cmath>
#include <cstring>  // For memset and memcpy
#include "iostream"

InputController::InputController()
    : m_mousePosition(0.f, 0.f)
    , m_mousePressed(false)
    , m_previousMousePressed(false)
    , m_activeDevice(InputDevice::Keyboard)
{
    initializeDefaultBindings();

    // Check if gamepad is already connected at startup
    detectActiveDevice();
}

void InputController::initializeDefaultBindings()
{
    // Gameplay
    m_keyBindings[InputAction::MoveUp] = sf::Keyboard::Key::W;
    m_keyBindings[InputAction::MoveDown] = sf::Keyboard::Key::S;
    m_keyBindings[InputAction::MoveLeft] = sf::Keyboard::Key::A;
    m_keyBindings[InputAction::MoveRight] = sf::Keyboard::Key::D;
    m_keyBindings[InputAction::Sprint] = sf::Keyboard::Key::LShift;

    // Menu navigation
    m_keyBindings[InputAction::Pause] = sf::Keyboard::Key::P;
    m_keyBindings[InputAction::Menu] = sf::Keyboard::Key::Escape;
    m_keyBindings[InputAction::Confirm] = sf::Keyboard::Key::Enter;
    m_keyBindings[InputAction::Cancel] = sf::Keyboard::Key::Escape;
}

// Check is a gamepad or keyboard is connected
void InputController::detectActiveDevice()
{
    // Check if gamepad 0 is connected
    if (sf::Joystick::isConnected(m_activeGamepad))
    {
        m_activeDevice = InputDevice::Gamepad;
    }
    else
    {
        m_activeDevice = InputDevice::Keyboard;
    }
}

/// <summary>
/// Updating the input states and events
/// 
/// Flow: 
/// 1: Save current state to previous state (for comparisons)
/// 2: Clear Current state
/// 3: Check if a gamepad was connected
/// 4: Update only what device is connected
/// </summary>
void InputController::update(const sf::Window& window)
{
    // ===== STEP 1: SAVE PREVIOUS STATE =====
    // Copy current -> previous
    // Example: If W was pressed last frame and this frame,
    //          wasJustPressed() returns false
    std::memcpy(m_previousState, m_currentState, sizeof(m_currentState));
    m_previousMousePressed = m_mousePressed;

    // ===== STEP 2: CLEAR CURRENT STATE =====
    // Reset all actions to false
    std::memset(m_currentState, 0, sizeof(m_currentState));

    // ===== STEP 3: DETECT DEVICE CHANGES =====
    // Check if gamepad was plugged in or unplugged
    detectActiveDevice();

    // ===== STEP 4: POLL ACTIVE INPUT DEVICE =====
    // Only check the device that's currently active
    switch (m_activeDevice)
    {
    case InputDevice::Gamepad:
        updateGamepad();
        break;
    case InputDevice::Keyboard:
        updateKeyboard();
        break;
    default: std::cout << "Unkown Input Device\n";
    }

    // Always update mouse (used in both modes, just in case as a fallback)
    updateMouse(window);
}

/*
*UPDATE KEYBOARD
*
* Polls keyboard input and updates m_currentState array.
* Only called when InputDevice::Keyboard is active.
*
* Iterates through all key bindings and checks if each key is pressed.
* Example: If W is bound to MoveUp and W is pressed,
* m_currentState[MoveUp] = true
*/
void InputController::updateKeyboard()
{
    // Check all key bindings
    for (const auto& [action, key] : m_keyBindings)
    {
        if (sf::Keyboard::isKeyPressed(key))
        {
            m_currentState[static_cast<int>(action)] = true;
        }
    }
}

/*
 * UPDATE GAMEPAD
 *
 * Polls gamepad input (analog sticks + buttons) and updates m_currentState.
 * Only called when InputDevice::Gamepad is active.
 *
 * Note: Analog stick values are converted to digital (on/off) actions.
 */
void InputController::updateGamepad()
{
    // ===== ANALOG STICK -> DIGITAL ACTIONS =====
    // Get stick position (-100 to +100, SFML's range)
    float range = 100.0f;
    float x = sf::Joystick::getAxisPosition(m_activeGamepad, sf::Joystick::Axis::X) / range;
    float y = sf::Joystick::getAxisPosition(m_activeGamepad, sf::Joystick::Axis::Y) / range;

    // Apply deadzone (ignore small movements from stick drift)
    x = applyDeadzone(x);
    y = applyDeadzone(y);

    // Convert analog to digital (threshold = 0.5)
    // Example: If stick is pushed 60% left, x = -0.6, MoveLeft = true
    float threeshold = 0.5f;
    if (x < -threeshold) m_currentState[static_cast<int>(InputAction::MoveLeft)] = true;
    if (x > threeshold)  m_currentState[static_cast<int>(InputAction::MoveRight)] = true;
    if (y < -threeshold) m_currentState[static_cast<int>(InputAction::MoveUp)] = true;
    if (y > threeshold)  m_currentState[static_cast<int>(InputAction::MoveDown)] = true;

    // ===== GAMEPAD BUTTONS =====
    // Button mapping (Xbox):
    // 0 = A/Cross (Confirm)
    // 1 = B/Circle (Cancel)
    // 7 = Start (Pause)
    if (sf::Joystick::isButtonPressed(m_activeGamepad, 0))
        m_currentState[static_cast<int>(InputAction::Confirm)] = true;
    if (sf::Joystick::isButtonPressed(m_activeGamepad, 1))
        m_currentState[static_cast<int>(InputAction::Cancel)] = true;
    if (sf::Joystick::isButtonPressed(m_activeGamepad, 7))
        m_currentState[static_cast<int>(InputAction::Pause)] = true;
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