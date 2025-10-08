#ifndef INPUT_CONTROLLER_H
#define INPUT_CONTROLLER_H

#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Joystick.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Window.hpp>
#include <unordered_map>


// Sources for information:
/* 
Based on the idea from https ://gameprogrammingpatterns.com/state.html
Map revision Cherno: https://youtu.be/KiB0vRi2wlc?si=c__NeniIIIjXwTsD
*/ 
// Memcpy and memset revision
/*
    // Memcpy: https://youtu.be/yoa_mMmvlMc?si=P-lZQa0Q1MZcHKVj
    // Memset: https://youtu.be/Njsn5HAnAnk?si=rmWTp8hw30NPpcCV
*/

// Personal Notes:
/// <summary>
/// Handles Keyboard and basic gamepad
/// 
/// Big O reminder:
/// O(1) doesnt change despite data size, O(n) performance can be affects as data increases
/// 
/// 1)Arrays for state storage (m_currentState/m_previousState)
///   O(1) lookup with no hashing
/// 
/// 2) unordered_map for key bindings (m_keyBindings)
///    Example:
///     If using map we have to do a comparison i.e Step 1: Compare Fire < MoveUp ? YES, go left
///     Using an unorderd map used hash i.e Hash Move->bucket 9 then go through map to get 9
/// 
/// 3) memset - Sets a block of memory to a specific value(byte by byte)
///     memset(pointer, value, size_in_bytes)
///     -Faster than looping through each element
/// 
///     Alternative without memset(orignal implementation) : - this is bad since I used to set values instead of direct address manipulation
///         for (int i = 0; i < static_cast<int>(InputAction::COUNT); ++i) {
///         m_currentState[i] = false;
///         m_previousState[i] = false;
/// 
/// 4) memcopy - Copies a block of memory data to another declared variable
///     memcopy(pDestination, pSource, size_in_bytes)
///     -Faster than looping through each element
/// 
///     Alternative without memcpy(orignal implementation) : - this is bad since I used to copy values instead of direct address manipulation
///         for (int i = 0; i < static_cast<int>(InputAction::COUNT); ++i) {
///         m_previousState[i] = m_currentState[i];
/// 
/// </summary>

enum class InputAction
{
    MoveUp, MoveDown, MoveLeft, MoveRight,
    Pause, Menu, Confirm, Cancel,
    COUNT // For enum size
};

class InputController
{
public:
    InputController();

    void update(const sf::Window& window);

    // ========== Action Queries (Keyboard/Gamepad) ==========
    // Current frame's input state
    bool isPressed(InputAction action) const;
    bool wasJustPressed(InputAction action) const;   // Transitioned from X to Y this frame
    bool wasJustReleased(InputAction action) const;  // Transitioned from Y to Z this frame

    // ========== Analog Input ==========
    // Get combined input as normalized axis (-1.0 to 1.0)
    float getHorizontalAxis() const;
    float getVerticalAxis() const;

    // ========== Mouse Input ==========
    bool isMousePressed() const { return m_mousePressed; }
    bool wasMouseClicked() const { return m_mouseClicked; }      // Just pressed this frame
    bool wasMouseReleased() const { return m_mouseReleased; }    // Just released this frame
    sf::Vector2f getMousePosition() const { return m_mousePosition; }  // Get current mouse position

    // ========== Configuration ==========
    // Runtime key rebinding
    void bindKey(InputAction action, sf::Keyboard::Key key);
    sf::Keyboard::Key getKeyBinding(InputAction action) const;
    void setGamepadDeadzone(float deadzone) { m_deadzone = deadzone; }

private:
    void initializeDefaultBindings();
    void updateKeyboardGamepad();
    void updateMouse(const sf::Window& window);
    float applyDeadzone(float value) const;

    // Array look ups for state
    bool m_currentState[static_cast<int>(InputAction::COUNT)];
    bool m_previousState[static_cast<int>(InputAction::COUNT)];

    // Hash map for rebindable keys
    std::unordered_map<InputAction, sf::Keyboard::Key> m_keyBindings;

    // Mouse state
    sf::Vector2f m_mousePosition;
    bool m_mousePressed;
    bool m_previousMousePressed;
    bool m_mouseClicked;    // Transitioned from unpressed to pressed
    bool m_mouseReleased;   // Transitioned from pressed to unpressed

    // Gamepad settings
    unsigned int m_activeGamepad = 0;
    float m_deadzone = 0.15f;  // Ignore joystick values below this threshold
};


// ========== Inline Implementations ==========
// Inline functions to be placed exactly where called

// TRUE only if: currently pressed AND was pressed last frame
inline bool InputController::isPressed(InputAction action) const
{
    return m_currentState[static_cast<int>(action)];
}

// TRUE only if: currently pressed AND was not pressed last frame
inline bool InputController::wasJustPressed(InputAction action) const
{
    int inputIndex = static_cast<int>(action);
    return m_currentState[inputIndex] && !m_previousState[inputIndex];
}

// TRUE only if: currently released AND was pressed last frame
inline bool InputController::wasJustReleased(InputAction action) const
{
    int inputIndex = static_cast<int>(action);
    return !m_currentState[inputIndex] && m_previousState[inputIndex];
}

#endif