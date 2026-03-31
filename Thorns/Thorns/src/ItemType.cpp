#include "ItemType.h"

ItemTypeRegistry::ItemTypeRegistry()
{
    loadDefaults();
}

const ItemTypeData* ItemTypeRegistry::get(ItemType type) const
{
    auto it = m_data.find(type);
    if (it != m_data.end())
        return &it->second;
    return nullptr;
}

bool ItemTypeRegistry::has(ItemType type) const
{
    return m_data.find(type) != m_data.end();
}

void ItemTypeRegistry::registerType(ItemType type, const ItemTypeData& data)
{
    m_data[type] = data;
}

void ItemTypeRegistry::loadDefaults()
{
    // ========== FOOD ==========
    ItemTypeData food;
    food.name = "Food";
    food.atlasRect = sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(64, 64));
    food.spriteSize = sf::Vector2f(32.f, 32.f); 
    food.statRestoreAmount = 30.f;                  
    food.pickupRadius = 32.f;                       
    registerType(ItemType::Food, food);

    // ========== WATER ==========
    ItemTypeData water;
    water.name = "Water";
    water.atlasRect = sf::IntRect(sf::Vector2i(64, 0), sf::Vector2i(64, 64));
    water.spriteSize = sf::Vector2f(32.f, 32.f);
    water.statRestoreAmount = 40.f;   
    water.pickupRadius = 32.f;
    registerType(ItemType::Water, water);
}