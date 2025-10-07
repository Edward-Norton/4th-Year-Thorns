#include "InputController.h"
#include <cmath>
#include <cstring>  // For memset and memcpy

InputController::InputController()
{
    /*
     * memset - Sets a block of memory to a specific value (byte by byte)
     *
     * memset(pointer, value, size_in_bytes)
     * - Sets all bytes in the array to 0 (false for bool)
     * - Faster than looping through each element
     * - Works because bool is typically 1 byte, and 0 represents false
     *
     * Alternative without memset:
     * for (int i = 0; i < static_cast<int>(InputAction::COUNT); ++i) {
     *     m_currentState[i] = false;
     *     m_previousState[i] = false;
     * }
     */
    std::memset(m_currentState, 0, sizeof(m_currentState));
    std::memset(m_previousState, 0, sizeof(m_previousState));

    initializeDefaultBindings();
}

void InputController::initializeDefaultBindings()
{
    // Set up default WASD + common keys
    // These can be changed at runtime via bindKey()
    m_keyBindings[InputAction::MoveUp] = sf::Keyboard::Key::W;
    m_keyBindings[InputAction::MoveDown] = sf::Keyboard::Key::S;
    m_keyBindings[InputAction::MoveLeft] = sf::Keyboard::Key::A;
    m_keyBindings[InputAction::MoveRight] = sf::Keyboard::Key::D;
    m_keyBindings[InputAction::Pause] = sf::Keyboard::Key::P;
    m_keyBindings[InputAction::Menu] = sf::Keyboard::Key::Escape;
    m_keyBindings[InputAction::Confirm] = sf::Keyboard::Key::Enter;
    m_keyBindings[InputAction::Cancel] = sf::Keyboard::Key::Escape;
}

void InputController::update()
{
    /*
     * Three-step update process:
     * 1. Save current state as previous (for transition detection)
     * 2. Clear current state (prepare for new hardware reading)
     * 3. Read hardware state into current
     */

     // Step 1: Save current state to previous
     /*
      * memcpy - Copies a block of memory from source to destination
      *
      * memcpy(destination, source, size_in_bytes)
      * - Copies the entire array in one operation
      * - Much faster than element-by-element copying
      * - Safe here because we're copying same-sized arrays
      *
      * Alternative without memcpy:
      * for (int i = 0; i < static_cast<int>(InputAction::COUNT); ++i) {
      *     m_previousState[i] = m_currentState[i];
      * }
      */
    std::memcpy(m_previousState, m_currentState, sizeof(m_currentState));

    // Step 2: Clear current state (set all to false/unpressed)
    std::memset(m_currentState, 0, sizeof(m_currentState));

    // Step 3: Read hardware and set current state
    // Check keyboard inputs
    for (const auto& [action, key] : m_keyBindings)
    {
        if (sf::Keyboard::isKeyPressed(key))
        {
            m_currentState[static_cast<int>(action)] = true;
        }
    }

    // Check gamepad inputs (if connected)
    if (sf::Joystick::isConnected(m_activeGamepad))
    {
        // Read analog stick position (-100 to +100, normalize to -1.0 to +1.0)
        float x = sf::Joystick::getAxisPosition(m_activeGamepad, sf::Joystick::Axis::X) / 100.0f;
        float y = sf::Joystick::getAxisPosition(m_activeGamepad, sf::Joystick::Axis::Y) / 100.0f;

        // Apply deadzone to prevent drift from worn joysticks
        x = applyDeadzone(x);
        y = applyDeadzone(y);

        // Convert analog values to digital inputs (threshold at 50%)
        if (x < -0.5f) m_currentState[static_cast<int>(InputAction::MoveLeft)] = true;
        if (x > 0.5f) m_currentState[static_cast<int>(InputAction::MoveRight)] = true;
        if (y < -0.5f) m_currentState[static_cast<int>(InputAction::MoveUp)] = true;
        if (y > 0.5f) m_currentState[static_cast<int>(InputAction::MoveDown)] = true;

        if (sf::Joystick::isButtonPressed(m_activeGamepad, 7)) // Start button
            m_currentState[static_cast<int>(InputAction::Pause)] = true;
    }
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
    /*
     * Deadzone prevents joystick drift
     *
     * Problem: Worn joysticks may report small values (0.05) when centered
     * Solution: Ignore values below threshold and rescale the rest
     *
     * Example with 0.15 deadzone:
     * - Input 0.10 -> Output 0.0 (ignored)
     * - Input 0.15 -> Output 0.0 (edge of deadzone)
     * - Input 0.50 -> Output ~0.41 (rescaled)
     * - Input 1.00 -> Output 1.0 (full input preserved)
     */
    if (std::abs(value) < m_deadzone)
        return 0.0f;

    // Rescale so deadzone edge maps to 0.0 and max input stays at 1.0
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