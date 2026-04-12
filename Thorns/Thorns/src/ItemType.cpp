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
    

    
    ItemTypeData food;
    food.itemType = ItemType::Food;
    food.useCategory = ItemUseCategory::Consume;
    food.name = "Food";
    food.atlasKey = "food-tin";
    food.spriteSize = sf::Vector2f(32.f, 32.f);  
    food.statRestoreAmount = 30.f;               
    food.pickupRadius = 32.f;
    registerType(ItemType::Food, food);

    
    ItemTypeData water;
    water.itemType = ItemType::Water;
    water.useCategory = ItemUseCategory::Consume;
    water.name = "Water";
    water.atlasKey = "water-bottle";
    water.spriteSize = sf::Vector2f(32.f, 32.f);
    water.statRestoreAmount = 40.f;               
    water.pickupRadius = 32.f;
    registerType(ItemType::Water, water);

    
    ItemTypeData firstAid;
    firstAid.itemType = ItemType::FirstAid;
    firstAid.useCategory = ItemUseCategory::Consume;
    firstAid.name = "First Aid";
    firstAid.atlasKey = "medkit";
    firstAid.spriteSize = sf::Vector2f(32.f, 32.f);
    firstAid.statRestoreAmount = 100.f;  
    firstAid.pickupRadius = 32.f;
    registerType(ItemType::FirstAid, firstAid);

    
    ItemTypeData bandage;
    bandage.itemType = ItemType::Bandage;
    bandage.useCategory = ItemUseCategory::Consume;
    bandage.name = "Bandage";
    bandage.atlasKey = "bandage";
    bandage.spriteSize = sf::Vector2f(32.f, 32.f);
    bandage.statRestoreAmount = 35.f;
    bandage.pickupRadius = 32.f;
    registerType(ItemType::Bandage, bandage);

    
    ItemTypeData knife;
    knife.itemType = ItemType::Knife;
    knife.useCategory = ItemUseCategory::Equip;
    knife.name = "Knife";
    knife.atlasKey = "knife";
    knife.spriteSize = sf::Vector2f(32.f, 32.f);
    knife.damage = 25.f;
    knife.pickupRadius = 32.f;
    registerType(ItemType::Knife, knife);

    
    ItemTypeData axe;
    axe.itemType = ItemType::Axe;
    axe.useCategory = ItemUseCategory::Equip;
    axe.name = "Axe";
    axe.atlasKey = "fire-axe";
    axe.spriteSize = sf::Vector2f(32.f, 32.f);
    axe.damage = 50.f;
    axe.pickupRadius = 32.f;
    registerType(ItemType::Axe, axe);

    
    ItemTypeData gun;
    gun.itemType = ItemType::Gun;
    gun.useCategory = ItemUseCategory::Equip;
    gun.name = "Gun";
    gun.atlasKey = "pistol";
    gun.spriteSize = sf::Vector2f(32.f, 32.f);
    gun.damage = 75.f;
    gun.pickupRadius = 32.f;
    registerType(ItemType::Gun, gun);
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
    
    
    static const std::pair<const char*, ItemType> table[] =
    {
        { "food-tin",      ItemType::Food   },
        { "water-bottle",  ItemType::Water  },
        { "medkit",   ItemType::FirstAid },
        { "bandage",     ItemType::Bandage  },
        { "knife",       ItemType::Knife    },
        { "fire-axe",         ItemType::Axe      },
        { "pistol",         ItemType::Gun      },
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

        
        ItemType type;
        if (!keyToType(name, type))
            continue;   

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