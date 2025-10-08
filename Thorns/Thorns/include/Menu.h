#ifndef MENU_HPP
#define MENU_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include "ButtonComponent.h"

// Personal Note:
// Forward declaration - we only need a pointer/reference, not the full definition
// This avoids circular dependencies and speeds up compilation
class InputController;

class Menu
{
public:
    Menu();
    ~Menu() = default;

    // ========== Initialization ==========
    bool initialize(const std::string& fontPath);

    // ========== Configuration ==========
    void setTitle(const std::string& title);
    void setPosition(const sf::Vector2f& pos);
    void setVisible(bool visible) { m_visible = visible; }
    void setButtonSpacing(float spacing) { m_buttonSpacing = spacing; }

    // ========== Button Management ==========
    void addButton(const std::string& text, std::function<void()> callback);
    void clearButtons();

    // ========== Update & Render ==========
    void update(const InputController& input);
    void render(sf::RenderTarget& target) const;

    // ========== State Queries ==========
    bool isVisible() const { return m_visible; }

private:
    void updateLayout();  // Recalculate button positions

    sf::Font m_font;
    std::unique_ptr<sf::Text> m_titleText;
    std::vector<std::unique_ptr<ButtonComponent>> m_buttons;

    sf::Vector2f m_position{ 800.f, 300.f };  // Top-left corner of menu
    float m_buttonSpacing = 60.f;           // Vertical space between buttons
    bool m_visible = true;
};

#endif