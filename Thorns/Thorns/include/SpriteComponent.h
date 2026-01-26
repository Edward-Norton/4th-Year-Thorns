#ifndef SPRITE_COMPONENT_HPP
#define SPRITE_COMPONENT_HPP

#include <SFML/Graphics.hpp>
#include <string>
#include "IRenderable.h"
#include "IPositionable.h"

/// <summary>
/// Component that manages sprite rendering and texture management
/// Supports both full textures and texture atlas regions
/// </summary>
class SpriteComponent : public IRenderable, public IPositionable
{
public:
    SpriteComponent();
    ~SpriteComponent() = default;

    // ========== Texture Loading ==========

    // Load full texture and scale to desired size
    bool loadTexture(const std::string& texturePath, float width = 64.f, float height = 64.f);

    // Load texture region from atlas (for sprite sheets)
    bool loadTexture(const std::string& texturePath, float width, float height, const sf::IntRect& textureRect);


    bool setSharedTexture(const sf::Texture& sharedTexture, float width, float height, const sf::IntRect& textureRect);

    // ========== IRenderable ==========
    void render(sf::RenderTarget& target) const override;

    // ========== IPositionable ==========
    void setPosition(const sf::Vector2f& pos) override;
    sf::Vector2f getPosition() const;

    // ========== Rotation ==========
    void setRotation(sf::Angle angle);
    sf::Angle getRotation() const;

    // ========== Origin ==========
    void setOrigin(const sf::Vector2f& origin);
    void centerOrigin();

    // ========== Size/Scale ==========
    void setSize(float width, float height);
    sf::Vector2f getSize() const;
    void setScale(const sf::Vector2f& scale);

    // ========== Movement ==========
    void move(const sf::Vector2f& offset);

    // ========== Texture Rect (for animation) ==========
    void setTextureRect(const sf::IntRect& rect);
    sf::IntRect getTextureRect() const;

    // ========== Collision ==========
    sf::FloatRect getBounds() const;
    sf::FloatRect getLocalBounds() const;

    // ========== Validation ==========
    bool isValid() const { return m_isValid; }

    // ========== Getters ==========
    sf::Sprite& getSprite() { return m_sprite; }
    const sf::Sprite& getSprite() const { return m_sprite; }

private:
    sf::Texture m_texture;
    sf::Sprite m_sprite;
    bool m_isValid;
    sf::IntRect m_textureRect;  // Store for animation switching
    sf::Vector2f m_targetSize;
};

#endif