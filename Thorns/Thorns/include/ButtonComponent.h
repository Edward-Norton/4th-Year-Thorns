#ifndef BUTTON_COMPONENT_HPP
#define BUTTON_COMPONENT_HPP

#include <SFML/Graphics.hpp>
#include <string>
#include <functional>
#include "IRenderable.h"
#include "IPositionable.h"

enum class ButtonState
{
    Normal,
    Hovered,
    Pressed,
    Disabled,
    Selected
};
 
class ButtonComponent : public IRenderable, public IPositionable
{
public:
    ButtonComponent();
    ~ButtonComponent() = default;

    
    bool initialize(const sf::Font& font, const std::string& text);

    
    void update(const sf::Vector2f& mousePos, bool mousePressed);

    
    void render(sf::RenderTarget& target) const override;

    
    
    void setPosition(const sf::Vector2f& pos) override;
    sf::Vector2f getPosition() const override;
    void setSize(const sf::Vector2f& size);
    void setText(const std::string& text);
    void setCallback(std::function<void()> callback);
    void setEnabled(bool enabled);
    void activate();
    void setSelected(bool selected);
    bool isSelected() const { return m_selected; }

    
    void setNormalColor(const sf::Color& color) { m_normalColor = color; }
    void setHoverColor(const sf::Color& color) { m_hoverColor = color; }
    void setPressedColor(const sf::Color& color) { m_pressedColor = color; }
    void setDisabledColor(const sf::Color& color) { m_disabledColor = color; }
    void setSelectedColor(const sf::Color& color) { m_selectedColor = color; }
    void setTextColor(const sf::Color& color);
    void setOutlineColor(const sf::Color& color);
    void setOutlineThickness(float thickness);

    
    bool isHovered() const { return m_state == ButtonState::Hovered; }
    bool isPressed() const { return m_wasClicked; }
    bool isEnabled() const { return m_enabled; }
    sf::FloatRect getBounds() const { return m_shape.getGlobalBounds(); }

private:
    void updateTextPosition();
    void updateColors();

    sf::RectangleShape m_shape;
    std::unique_ptr<sf::Text> m_text;
    std::function<void()> m_callback;

    ButtonState m_state;
    bool m_enabled;
    bool m_wasClicked;
    bool m_previousMousePressed;
    bool m_selected;

    
    sf::Color m_normalColor;
    sf::Color m_hoverColor;
    sf::Color m_pressedColor;
    sf::Color m_disabledColor;
    sf::Color m_selectedColor;
    sf::Color m_textColor;
};

#endif