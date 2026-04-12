#ifndef SPRITE_COMPONENT_HPP
#define SPRITE_COMPONENT_HPP

#include <SFML/Graphics.hpp>
#include <string>
#include "IRenderable.h"
#include "IPositionable.h"

class SpriteComponent : public IRenderable, public IPositionable
{
public:
    SpriteComponent();
    ~SpriteComponent() = default;

    

    
    bool loadTexture(const std::string& texturePath, float width = 64.f, float height = 64.f);

    
    bool loadTexture(const std::string& texturePath, float width, float height, const sf::IntRect& textureRect);

    bool setSharedTexture(const sf::Texture& sharedTexture, float width, float height, const sf::IntRect& textureRect);

    
    void render(sf::RenderTarget& target) const override;

    
    void setPosition(const sf::Vector2f& pos) override;
    sf::Vector2f getPosition() const;

    
    void setRotation(sf::Angle angle);
    sf::Angle getRotation() const;

    
    void setOrigin(const sf::Vector2f& origin);
    void centerOrigin();

    
    void setSize(float width, float height);
    sf::Vector2f getSize() const;
    void setScale(const sf::Vector2f& scale);

    
    void move(const sf::Vector2f& offset);

    
    void setTextureRect(const sf::IntRect& rect);
    sf::IntRect getTextureRect() const;

    
    sf::FloatRect getBounds() const;
    sf::FloatRect getLocalBounds() const;

    
    bool isValid() const { return m_isValid; }

    
    sf::Sprite& getSprite() { return m_sprite; }
    const sf::Sprite& getSprite() const { return m_sprite; }

private:
    sf::Texture m_texture;
    sf::Sprite m_sprite;
    bool m_isValid;
    sf::IntRect m_textureRect;  
    sf::Vector2f m_targetSize;
};

#endif