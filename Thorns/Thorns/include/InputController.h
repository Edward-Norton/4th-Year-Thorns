#ifndef INPUT_CONTROLLER_H
#define INPUT_CONTROLLER_H

#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Joystick.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Window.hpp>
#include <unordered_map>

 

enum class InputAction
{
    
    MoveUp, MoveDown, MoveLeft, MoveRight, Sprint,  
    Inventory,

    
    Pause, Menu, Confirm, Cancel,

    
    COUNT
};

enum class InputDevice
{
    Keyboard,   
    Gamepad     
};

class InputController
{
public:
    InputController();

    void update(const sf::Window& window);

    
    
    bool isPressed(InputAction action) const;
    bool wasJustPressed(InputAction action) const;   
    bool wasJustReleased(InputAction action) const;  

    
    bool isMousePressed() const { return m_leftMousePressed; }
    bool wasMouseJustPressed() const;
    bool wasMouseJustReleased() const;
    sf::Vector2f getMousePosition() const { return m_mousePosition; }  

    bool isRightMousePressed() const { return m_rightMousePressed; }
    bool wasRightMouseJustPressed() const;
    bool wasRightMouseJustReleased() const;

    
    
    void bindKey(InputAction action, sf::Keyboard::Key key);
    sf::Keyboard::Key getKeyBinding(InputAction action) const;
    void setGamepadDeadzone(float deadzone) { m_deadzone = deadzone; }

    
    InputDevice getActiveDevice() const { return m_activeDevice; }
    bool isGamepadConnected() const { return m_activeDevice == InputDevice::Gamepad; }

private:
    void initializeDefaultBindings();
    void detectActiveDevice();        
    void updateKeyboard();
    void updateGamepad();
    void updateMouse(const sf::Window& window);
    float applyDeadzone(float value) const;

    
    
    bool m_currentState[static_cast<int>(InputAction::COUNT)];
    bool m_previousState[static_cast<int>(InputAction::COUNT)];

    
    
    std::unordered_map<InputAction, sf::Keyboard::Key> m_keyBindings;

    
    sf::Vector2f m_mousePosition;
    bool m_leftMousePressed;
    bool m_previousLeftMousePressed;
    bool m_rightMousePressed;
    bool m_previousRightMousePressed;

    
    sf::Vector2f m_rightStickAxis;

    
    InputDevice m_activeDevice = InputDevice::Keyboard;  
    unsigned int m_activeGamepad = 0;
    float m_deadzone = 0.15f;  
};

inline bool InputController::isPressed(InputAction action) const
{
    return m_currentState[static_cast<int>(action)];
}

inline bool InputController::wasJustPressed(InputAction action) const
{
    int inputIndex = static_cast<int>(action);
    return m_currentState[inputIndex] && !m_previousState[inputIndex];
}

inline bool InputController::wasJustReleased(InputAction action) const
{
    int inputIndex = static_cast<int>(action);
    return !m_currentState[inputIndex] && m_previousState[inputIndex];
}

inline bool InputController::wasMouseJustPressed() const
{
    return m_leftMousePressed && !m_previousLeftMousePressed;
}

inline bool InputController::wasMouseJustReleased() const
{
    return !m_leftMousePressed && m_previousLeftMousePressed;
}

inline bool InputController::wasRightMouseJustPressed() const
{
    return m_rightMousePressed && !m_previousRightMousePressed;
}

inline bool InputController::wasRightMouseJustReleased() const
{
    return !m_rightMousePressed && m_previousRightMousePressed;
}

#endif