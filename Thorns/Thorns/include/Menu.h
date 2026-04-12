#ifndef MENU_HPP
#define MENU_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include "ButtonComponent.h"

class InputController;

class Menu
{
public:
    Menu();
    ~Menu() = default;

    
    bool initialize(const std::string& fontPath);

    
    void setTitle(const std::string& title);
    void setPosition(const sf::Vector2f& pos);
    void setVisible(bool visible) { m_visible = visible; }
    void setButtonSpacing(float spacing) { m_buttonSpacing = spacing; }

    
    void addButton(const std::string& text, std::function<void()> callback);
    void clearButtons();

    
    void update(const InputController& input);
    void render(sf::RenderTarget& target) const;

    
    bool isVisible() const { return m_visible; }

private:
    void updateLayout();  

    void handleKeyboardGamepadInput(const InputController& input);
    void handleMouseInput(const InputController& input);
    void updateButtonVisuals(const InputController& input);

    sf::Font m_font;
    std::unique_ptr<sf::Text> m_titleText;
    std::vector<std::unique_ptr<ButtonComponent>> m_buttons;

    sf::Vector2f m_position;  
    float m_buttonSpacing;           
    bool m_visible = true;
    int m_selectedIndex = 0;
};

#endif