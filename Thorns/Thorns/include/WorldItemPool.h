#ifndef WORLD_ITEM_POOL_H
#define WORLD_ITEM_POOL_H

#include <array>
#include <vector>
#include "WorldItem.h"
#include "ItemType.h"

// ========== WORLD ITEM POOL ==========
// Allocate MAX_ITEMS WorldItem slots to heap allocation at run time for pointer crashes if occured. 
class WorldItemPool
{
public:
    // Max simultaneous world items.
    static constexpr int MAX_ITEMS = 64;

    WorldItemPool();
    ~WorldItemPool() = default;

    // ========== Initialization ==========

    // Loads the shared atlas texture once.
    bool initialize(const std::string& atlasPath);

    // ========== Spawning ==========
    // Activates the first available pool slot at worldPos for the given type.
    WorldItem* spawn(ItemType type, const sf::Vector2f& worldPos,
        const ItemTypeRegistry& registry);

    // Returns a slot back to the pool.
    void despawn(WorldItem* item);

    // Deactivates all slots. PN: Call on map regen
    void despawnAll();

    // ========== Rendering ==========

    // Renders all active items with the frustum culling.
    void render(sf::RenderTarget& target, const sf::View& view) const;

    // ========== Queries ==========

    // Returns non-owning pointers to all currently active items.
    // Called each frame by Game for pickup collision checks.
    std::vector<WorldItem*> getActiveItems();

    int getActiveCount() const;
    bool isInitialized() const { return m_atlasLoaded; }

private:
    std::array<WorldItem, MAX_ITEMS> m_pool;

    sf::Texture m_atlas;
    bool m_atlasLoaded;
};

#endif