#ifndef SPRITE_COMPONENT_HPP
#define SPRITE_COMPONENT_HPP

#include <SFML/Graphics.hpp>
#include <string>

// Component that manages sprite and texture
class SpriteComponent
{
public:
    SpriteComponent();
    ~SpriteComponent() = default;

    bool loadTexture(const std::string& texturePath, float width = 64.f, float height = 64.f);

    void render(sf::RenderTarget& target) const;

    // Position
    void setPosition(const sf::Vector2f& pos);
    sf::Vector2f getPosition() const;

    // Rotation
    void setRotation(sf::Angle angle);
    sf::Angle getRotation() const;

    // Origin
    void setOrigin(const sf::Vector2f& origin);
    void centerOrigin();

    // Size/Scale
    void setSize(float width, float height);
    sf::Vector2f getSize() const;
    void setScale(const sf::Vector2f& scale);

    // Movement
    void move(const sf::Vector2f& offset);

    // Collision
    sf::FloatRect getBounds() const;
    sf::FloatRect getLocalBounds() const;

    bool isValid() const { return m_isValid; }

    // Direct sprite access if needed
    sf::Sprite& getSprite() { return m_sprite; }
    const sf::Sprite& getSprite() const { return m_sprite; }

private:
    sf::Texture m_texture;
    sf::Sprite m_sprite;
    bool m_isValid;
};

#endif