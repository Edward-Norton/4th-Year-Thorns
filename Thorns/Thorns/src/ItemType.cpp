#include "ItemType.h"
#include <fstream>
#include <sstream>
#include <iostream>

ItemTypeRegistry::ItemTypeRegistry()
{
    registerDefaults();
}

void ItemTypeRegistry::registerDefaults()
{
    // Gameplay data only

    // ========== FOOD ==========
    ItemTypeData food;
    food.name = "Food";
    food.atlasKey = "food-tin";
    food.spriteSize = sf::Vector2f(32.f, 32.f);  // World render size
    food.statRestoreAmount = 30.f;               // Restores 30 hunger on use
    food.pickupRadius = 32.f;
    registerType(ItemType::Food, food);

    // ========== WATER ==========
    ItemTypeData water;
    water.name = "Water";
    water.atlasKey = "water-bottle";
    water.spriteSize = sf::Vector2f(32.f, 32.f);
    water.statRestoreAmount = 40.f;               // Restores 40 water on use
    water.pickupRadius = 32.f;
    registerType(ItemType::Water, water);
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

bool ItemTypeRegistry::keyToType(const std::string& key, ItemType& outType)
{
    // Maps txt name column -> ItemType enum.
    // PN: Need to add the rest when ready
    static const std::pair<const char*, ItemType> table[] =
    {
        { "food-tin",      ItemType::Food  },
        { "water-bottle",  ItemType::Water },
    };

    for (const auto& [k, t] : table)
    {
        if (key == k)
        {
            outType = t;
            return true;
        }
    }
    return false;
}

bool ItemTypeRegistry::loadDefinitions(const std::string& definitionsPath)
{
    std::ifstream file(definitionsPath);
    if (!file.is_open())
    {
        std::cerr << "ItemTypeRegistry::loadDefinitions: Cannot open "
            << definitionsPath << "\n";
        return false;
    }

    int loaded = 0;
    std::string line;

    while (std::getline(file, line))
    {
        if (line.empty()) continue;

        // Format: name,x,y,width,height
        std::stringstream ss(line);
        std::string name;
        int x = 0, y = 0, w = 0, h = 0;
        char comma;

        if (!(std::getline(ss, name, ',')
            && ss >> x >> comma >> y >> comma >> w >> comma >> h))
        {
            std::cerr << "ItemTypeRegistry::loadDefinitions: Bad line: "
                << line << "\n";
            continue;
        }

        // Check if this key maps to a registered ItemType
        ItemType type;
        if (!keyToType(name, type))
            continue;   // Not an item to care about (for now until testing it done)

        auto it = m_data.find(type);
        if (it == m_data.end())
            continue;

        it->second.atlasRect = sf::IntRect(sf::Vector2i(x, y), sf::Vector2i(w, h));
        std::cout << "ItemTypeRegistry: Loaded rect for '" << name
            << "' [" << x << "," << y << " " << w << "x" << h << "]\n";
        ++loaded;
    }

    std::cout << "ItemTypeRegistry: " << loaded << " atlas rect(s) loaded from "
        << definitionsPath << "\n";
    return loaded > 0;
}