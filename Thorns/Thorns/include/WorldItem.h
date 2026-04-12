#ifndef WORLD_ITEM_H
#define WORLD_ITEM_H

#include <SFML/Graphics.hpp>
#include "IRenderable.h"
#include "IPositionable.h"
#include "ICollidable.h"
#include "SpriteComponent.h"
#include "ItemType.h"

class WorldItem : public IRenderable, public IPositionable, public ICollidable
{
public:
    WorldItem();
    ~WorldItem() = default;

    

    
    bool activate(ItemType type, const sf::Vector2f& worldPos,
        const ItemTypeRegistry& registry,
        const sf::Texture& sharedAtlas);

    
    void deactivate();

    bool isActive() const { return m_active; }

    
    void render(sf::RenderTarget& target) const override;

    
    void setPosition(const sf::Vector2f& pos) override;
    sf::Vector2f getPosition() const override { return m_worldPosition; }

    
    sf::FloatRect getBounds() const override;

    
    ItemType getType() const { return m_type; }

private:
    ItemType        m_type;
    sf::Vector2f    m_worldPosition;
    SpriteComponent m_sprite;
    bool            m_active;

    float           m_pickupRadius;
};

#endif