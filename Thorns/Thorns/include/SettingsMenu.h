/*
 * SettingsMenu - Key binding configuration UI
 *
 * Workflow:
 * 1. Display list of actions with current key bindings
 * 2. User clicks a button to rebind an action
 * 3. Menu enters "waiting for key" mode
 * 4. User presses a key (or ESC to cancel)
 * 5. Key is bound to the action and displayed
 *
 * Layout:
 * [Action Label] [Current Key]
 * Move Up         W
 * Move Down       S
 * ...
 * [Back Button]
 */

#ifndef SETTINGS_MENU_HPP
#define SETTINGS_MENU_HPP

#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>
#include <functional>
#include "ButtonComponent.h"
#include "InputController.h"

class SettingsMenu
{
public:
    SettingsMenu();
    ~SettingsMenu() = default;

    // ========== Initialization ==========
    bool initialize(const std::string& fontPath, InputController* inputController);

    // ========== Update & Render ==========
    void update(const sf::Vector2f& mousePos, bool mousePressed);
    void render(sf::RenderTarget& target) const;

    // ========== Key Rebinding ==========
    void handleKeyPress(sf::Keyboard::Key key);  // Called from Game when in rebind mode
    bool isWaitingForKey() const { return m_waitingForKey; }

    // ========== Visibility ==========
    void setVisible(bool visible) { m_visible = visible; }
    bool isVisible() const { return m_visible; }

    // ========== Callbacks ==========
    void setBackCallback(std::function<void()> callback) { m_backCallback = callback; }

private:
    // ========== Internal Structure ==========
    struct KeyBindingRow
    {
        InputAction action;                        // Which action this row configures
        std::unique_ptr<ButtonComponent> button;   // Button to click for rebinding
        std::unique_ptr<sf::Text> keyLabel;        // Displays current key (e.g., "W")
    };

    // ========== Helper Methods ==========
    void createKeyBindingRows();        // Build the UI rows
    void updateKeyLabels();             // Refresh displayed key names
    std::string getKeyName(sf::Keyboard::Key key) const;  // Convert enum to string

    // ========== Data ==========
    sf::Font m_font;
    InputController* m_inputController;  // Reference to input system (not owned)

    std::unique_ptr<sf::Text> m_titleText;
    std::vector<KeyBindingRow> m_keyBindingRows;  // One row per rebindable action
    std::unique_ptr<ButtonComponent> m_backButton;

    // ========== State ==========
    bool m_visible;
    bool m_waitingForKey;         // True when user has clicked a button and we're waiting for keypress
    InputAction m_actionToRebind; // Which action we're currently rebinding

    std::function<void()> m_backCallback;  // Called when back button is clicked

    // ========== Layout Constants ==========
    static constexpr float BUTTON_WIDTH = 250.f;
    static constexpr float BUTTON_HEIGHT = 45.f;
    static constexpr float BUTTON_SPACING = 10.f;
    static constexpr float START_Y = 150.f;
};

#endif