#ifndef CURSOR_COMPONENT_H
#define CURSOR_COMPONENT_H

#include <SFML/Graphics.hpp>
#include <SFML/System/Vector2.hpp>
#include "IRenderable.h"

class CursorComponent : public IRenderable
{
public:
    CursorComponent();
    ~CursorComponent() = default;

    
    bool initialize(float radius = 5.f);

    
    void update(const sf::Vector2f& mousePosition);

    
    void render(sf::RenderTarget& target) const override;

    
    sf::Vector2f getPosition() const { return m_position; }

    
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