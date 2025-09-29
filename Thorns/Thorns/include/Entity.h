#ifndef ENTITY_HPP
#define ENTITY_HPP

#include <SFML/Graphics.hpp>
#include <string>

class Entity
{
public:
    Entity();
    virtual ~Entity() = default;

    // Pure virtual methods that derived classes MUST implement
    virtual void update(sf::Time deltaTime) = 0;
    virtual void initTexture() = 0;  // Each derived class defines its own texture loading

    // Rendering for derived classes
    virtual void render(sf::RenderTarget& target) const;

    // Check if entity was initialized successfully
    bool isValid() const { return m_isValid; }

    // Position
    void setPosition(const sf::Vector2f& pos);
    sf::Vector2f getPosition() const;

    // Speed
    void setSpeed(float speed);
    float getSpeed() const;

    // Size
    void setSize(float width, float height);
    sf::Vector2f getSize() const;

    // Collision
    sf::FloatRect getBounds() const;

protected:
    sf::Texture m_texture;
    sf::Sprite m_sprite;
    float m_speed{ 100.f };
    bool m_isValid{ false };

    // Helper function for derived classes to use in their initTexture() implementations
    bool loadTexture(const std::string& texturePath, float width = 64.f, float height = 64.f); //Default to width and height values if not given.
};

#endif //ENTITY_HPP