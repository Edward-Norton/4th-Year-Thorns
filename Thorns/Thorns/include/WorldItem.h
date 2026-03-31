#ifndef WORLD_ITEM_H
#define WORLD_ITEM_H

#include <SFML/Graphics.hpp>
#include "IRenderable.h"
#include "IPositionable.h"
#include "ICollidable.h"
#include "SpriteComponent.h"
#include "ItemType.h"

// ========== WORLD ITEM ==========
// Collectable object
// Type, sprite and post
//
// PN: Object spawned via the pool
class WorldItem : public IRenderable, public IPositionable, public ICollidable
{
public:
    WorldItem();
    ~WorldItem() = default;

    // ========== Pool Lifecycle ==========

    // Activates this pool slot for a given type and position
    bool activate(ItemType type, const sf::Vector2f& worldPos,
        const ItemTypeRegistry& registry,
        const sf::Texture& sharedAtlas);

    // Returns this slot to the available state
    void deactivate();

    bool isActive() const { return m_active; }

    // ========== IRenderable ==========
    void render(sf::RenderTarget& target) const override;

    // ========== IPositionable ==========
    void setPosition(const sf::Vector2f& pos) override;
    sf::Vector2f getPosition() const override { return m_worldPosition; }

    // ========== ICollidable ==========
    sf::FloatRect getBounds() const override;

    // ========== Queries ==========
    ItemType getType() const { return m_type; }

private:
    ItemType        m_type;
    sf::Vector2f    m_worldPosition;
    SpriteComponent m_sprite;
    bool            m_active;

    float           m_pickupRadius;
};

#endif