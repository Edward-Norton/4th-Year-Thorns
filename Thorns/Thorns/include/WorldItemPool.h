#ifndef WORLD_ITEM_POOL_H
#define WORLD_ITEM_POOL_H

#include <array>
#include <vector>
#include "WorldItem.h"
#include "ItemType.h"

class WorldItemPool
{
public:
    
    static constexpr int MAX_ITEMS = 64;

    WorldItemPool();
    ~WorldItemPool() = default;

    

    
    bool initialize(const std::string& atlasPath);

    
    
    WorldItem* spawn(ItemType type, const sf::Vector2f& worldPos,
        const ItemTypeRegistry& registry);

    
    void despawn(WorldItem* item);

    
    void despawnAll();

    

    
    void render(sf::RenderTarget& target, const sf::View& view) const;

    

    
    
    std::vector<WorldItem*> getActiveItems();

    const sf::Texture& getAtlas() const { return m_atlas; }

    int getActiveCount() const;
    bool isInitialized() const { return m_atlasLoaded; }

private:
    std::array<WorldItem, MAX_ITEMS> m_pool;

    sf::Texture m_atlas;
    bool m_atlasLoaded;
};

#endif