#ifndef CURSOR_COMPONENT_H
#define CURSOR_COMPONENT_H

#include <SFML/Graphics.hpp>
#include <SFML/System/Vector2.hpp>

/// <summary>
/// Visual cursor that follows the mouse position
/// </summary>
class CursorComponent
{
public:
    CursorComponent();
    ~CursorComponent() = default;

    // Initialization
    bool initialize(float radius = 5.f);

    // Update cursor position to follow mouse
    void update(const sf::Vector2f& mousePosition);

    // Render cursor to screen
    void render(sf::RenderTarget& target) const;

    // Getters
    sf::Vector2f getPosition() const { return m_position; }

    // Visual customization
    void setColor(const sf::Color& color);
    void setRadius(float radius);
    void setVisible(bool visible) { m_visible = visible; }
    bool isVisible() const { return m_visible; }

private:
    sf::CircleShape m_cursorShape;
    sf::Vector2f m_position;
    bool m_visible;
    float m_radius;
};

#endif