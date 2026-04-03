#include "WorldItemPool.h"
#include <iostream>

WorldItemPool::WorldItemPool()
    : m_atlasLoaded(false)
{
    
}

bool WorldItemPool::initialize(const std::string& atlasPath)
{
    if (!m_atlas.loadFromFile(atlasPath))
    {
        std::cerr << "WorldItemPool::initialize: Failed to load atlas: "
            << atlasPath << "\n";
        m_atlasLoaded = false;
        return false;
    }

    m_atlasLoaded = true;
    std::cout << "WorldItemPool: Atlas loaded (" << atlasPath << ")\n";
    return true;
}

WorldItem* WorldItemPool::spawn(ItemType type, const sf::Vector2f& worldPos,
    const ItemTypeRegistry& registry)
{
    if (!m_atlasLoaded)
    {
        std::cerr << "WorldItemPool::spawn: Pool not initialized. "
            "Call initialize() before spawning.\n";
        return nullptr;
    }


    for (auto& item : m_pool)
    {
        if (!item.isActive())
        {
            if (item.activate(type, worldPos, registry, m_atlas))
                return &item;

            return nullptr;
        }
    }

    std::cerr << "WorldItemPool::spawn: Pool exhausted ("
        << MAX_ITEMS << " items active). "
        "Increase MAX_ITEMS if the map needs more pickups.\n";
    return nullptr;
}

void WorldItemPool::despawn(WorldItem* item)
{
    if (!item)
        return;

    item->deactivate();
}

void WorldItemPool::despawnAll()
{
    for (auto& item : m_pool)
        item.deactivate();
}

void WorldItemPool::render(sf::RenderTarget& target, const sf::View& view) const
{
    const sf::Vector2f centre = view.getCenter();
    const sf::Vector2f size = view.getSize();
    const sf::FloatRect viewBounds(
        sf::Vector2f(centre.x - size.x * 0.5f, centre.y - size.y * 0.5f),
        size
    );

    for (const auto& item : m_pool)
    {
        if (!item.isActive())
            continue;

        if (!viewBounds.findIntersection(item.getBounds()).has_value())
            continue;

        item.render(target);
    }
}

std::vector<WorldItem*> WorldItemPool::getActiveItems()
{
    std::vector<WorldItem*> active;
    active.reserve(MAX_ITEMS);

    for (auto& item : m_pool)
    {
        if (item.isActive())
            active.push_back(&item);
    }

    return active;
}

int WorldItemPool::getActiveCount() const
{
    int count = 0;
    for (const auto& item : m_pool)
    {
        if (item.isActive())
            ++count;
    }
    return count;
}