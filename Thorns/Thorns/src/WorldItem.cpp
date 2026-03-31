#include "WorldItem.h"
#include <iostream>

WorldItem::WorldItem()
    : m_type(ItemType::Food)
    , m_worldPosition(0.f, 0.f)
    , m_active(false)
    , m_pickupRadius(32.f)
{
}

bool WorldItem::activate(ItemType type, const sf::Vector2f& worldPos,
    const ItemTypeRegistry& registry,
    const sf::Texture& sharedAtlas)
{
    const ItemTypeData* data = registry.get(type);
    if (!data)
    {
        std::cerr << "WorldItem::activate: No data registered for ItemType "
            << static_cast<int>(type) << "\n";
        return false;
    }

    m_type = type;
    m_worldPosition = worldPos;
    m_pickupRadius = data->pickupRadius;


    if (!m_sprite.setSharedTexture(sharedAtlas,
        data->spriteSize.x,
        data->spriteSize.y,
        data->atlasRect))
    {
        std::cerr << "WorldItem::activate: setSharedTexture failed for type "
            << static_cast<int>(type) << "\n";
        return false;
    }

    m_sprite.centerOrigin();
    m_sprite.setPosition(m_worldPosition);
    m_active = true;
    return true;
}

void WorldItem::deactivate()
{
    m_active = false;
}

void WorldItem::render(sf::RenderTarget& target) const
{
    if (!m_active || !m_sprite.isValid())
        return;

    m_sprite.render(target);
}

void WorldItem::setPosition(const sf::Vector2f& pos)
{
    m_worldPosition = pos;
    m_sprite.setPosition(pos);
}

sf::FloatRect WorldItem::getBounds() const
{
    // AABB centred on world position
    return sf::FloatRect(
        sf::Vector2f(m_worldPosition.x - m_pickupRadius,
            m_worldPosition.y - m_pickupRadius),
        sf::Vector2f(m_pickupRadius * 2.f, m_pickupRadius * 2.f)
    );
}