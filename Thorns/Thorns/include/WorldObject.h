#ifndef WORLD_OBJECT_HPP
#define WORLD_OBJECT_HPP

#include <SFML/Graphics.hpp>
#include <string>
#include "SpriteComponent.h"
#include "IRenderable.h"
#include "IPositionable.h"
#include "ICollidable.h"

/// <summary>
/// Represents a procedurally placed object in the world (trees, rocks, etc.)
/// These are decorative objects generated using Perlin noise
/// 
/// Design:
/// - Lightweight: Only stores position and sprite reference
/// - Batched rendering: Multiple objects share same texture atlas
/// </summary>
class WorldObject : public IRenderable, public IPositionable, public ICollidable
{
public:
    /// Object type determines sprite and placement rules
    enum class Type
    {
        SmallRoot,      // Small tree roots (testing)
        TreeTop1,       // Tree canopy variant 1
        TreeTop2,       // Tree canopy variant 2
        LargeRoot,      // Large tree root system
        SmallRootBasic, // Basic small root
        COUNT           // Total number of types
    };

    WorldObject(Type type, const sf::Vector2f& worldPos);
    ~WorldObject() = default;

    // ========== IRenderable ==========
    void render(sf::RenderTarget& target) const override;

    // ========== IPositionable ==========
    sf::Vector2f getPosition() const override { return m_worldPosition; }
    void setPosition(const sf::Vector2f& pos) override;

    // ICollidable implementation
    sf::FloatRect getBounds() const override;

    // ========== Configuration ==========
    bool loadSpriteFromTexture(const sf::Texture& sharedTexture, const sf::IntRect& textureRect, const sf::Vector2f& size);

    // ========== Queries ==========
    Type getType() const { return m_type; }
    bool isValid() const;

private:
    Type m_type;
    sf::Vector2f m_worldPosition;
    std::unique_ptr<SpriteComponent> m_sprite;
};

#endif