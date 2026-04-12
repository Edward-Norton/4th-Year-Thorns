

#ifndef SETTINGS_MENU_HPP
#define SETTINGS_MENU_HPP

#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>
#include <functional>
#include "ButtonComponent.h"
#include "InputController.h"
#include "ScreenSettings.h"

class SettingsMenu
{
public:
    SettingsMenu();
    ~SettingsMenu() = default;

    
    bool initialize(const std::string& fontPath, InputController* inputController, ScreenSettings* screenSettings);

    
    void update(const sf::Vector2f& mousePos, bool mousePressed);
    void render(sf::RenderTarget& target) const;

    
    void handleKeyPress(sf::Keyboard::Key key);
    bool isWaitingForKey() const { return m_waitingForKey; }

    
    void setVisible(bool visible);
    bool isVisible() const { return m_visible; }

    
    void setBackCallback(std::function<void()> callback) { m_backCallback = callback; }
    void setApplyCallback(std::function<void()> callback) { m_applyCallback = callback; }

    
    void updateLayout(unsigned int screenWidth, unsigned int screenHeight);

private:
    
    struct KeyBindingRow
    {
        InputAction action;
        std::unique_ptr<ButtonComponent> button;
        std::unique_ptr<sf::Text> keyLabel;
    };

    
    void createKeyBindingSection();
    void createVideoSection();
    void updateKeyLabels();
    void updateVideoLabels();
    std::string getKeyName(sf::Keyboard::Key key) const;

    
    void onResolutionPrev();
    void onResolutionNext();
    void onFullscreenToggle();
    void onApply();
    void onBack();

    
    sf::Font m_font;
    InputController* m_inputController;
    ScreenSettings* m_screenSettings;

    
    std::unique_ptr<sf::Text> m_titleText;
    std::unique_ptr<sf::Text> m_keyBindingsHeader;
    std::unique_ptr<sf::Text> m_videoHeader;

    std::vector<KeyBindingRow> m_keyBindingRows;

    
    std::unique_ptr<sf::Text> m_resolutionLabel;
    std::unique_ptr<sf::Text> m_resolutionValue;
    std::unique_ptr<ButtonComponent> m_resolutionPrevBtn;
    std::unique_ptr<ButtonComponent> m_resolutionNextBtn;

    std::unique_ptr<sf::Text> m_fullscreenLabel;
    std::unique_ptr<ButtonComponent> m_fullscreenToggleBtn;

    std::unique_ptr<ButtonComponent> m_applyButton;
    std::unique_ptr<ButtonComponent> m_backButton;

    
    bool m_visible;
    bool m_waitingForKey;
    InputAction m_actionToRebind;

    std::function<void()> m_backCallback;
    std::function<void()> m_applyCallback;

    
    static constexpr float BUTTON_WIDTH = 250.f;
    static constexpr float BUTTON_HEIGHT = 45.f;
    static constexpr float BUTTON_SPACING = 10.f;
    static constexpr float SECTION_SPACING = 40.f;

    
    float m_startX;
    float m_startY;
    float m_keyBindingsY;
    float m_videoSettingsY;
};

#endif