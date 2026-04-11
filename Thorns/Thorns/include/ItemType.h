#ifndef ITEM_TYPE_H
#define ITEM_TYPE_H

#include <SFML/Graphics.hpp>
#include <string>
#include <unordered_map>

// ========== ITEM TYPES ==========
enum class ItemType
{
    // Consumables
    Food,
    Water,
    FirstAid,    // Restores health fully
    Bandage,     // Restores partial health

    // Weapons
    Knife,
    Axe,
    Gun,

    COUNT
};


enum class ItemUseCategory
{
    Consume,   // Remove from inventory after use
    Equip,     // Keep but use the item
};

// ========== FLYWEIGHT DATA ==========
// All of the items are to use this format, only need it once per object
struct ItemTypeData
{
    ItemType     itemType;
    ItemUseCategory useCategory = ItemUseCategory::Consume;
    std::string  name;               // Display name shown in inventory
    std::string  atlasKey;           // Key matching the name column in items_atlas.txt
    sf::IntRect  atlasRect;          // Region inside the shared items atlas
    sf::Vector2f spriteSize;         // World-space render size in pixels
    float        statRestoreAmount;  // Units of the stat restored on use
    float        damage;             // Damage to other entities
    float        pickupRadius;       // Pixel radius at which player triggers collection
};

// ========== ITEM TYPE REGISTRY ==========
class ItemTypeRegistry
{
public:
    ItemTypeRegistry();
    ~ItemTypeRegistry() = default;

    // Get the definitions from the asset file
    bool loadDefinitions(const std::string& definitionsPath);

    // Retrieve shared data for a type. Returns nullptr if type not registered.
    const ItemTypeData* get(ItemType type) const;

    // Returns true if type has registered data.
    bool has(ItemType type) const;

private:

    void registerDefaults();

    void registerType(ItemType type, const ItemTypeData& data);

    static bool keyToType(const std::string& key, ItemType& outType);

    std::unordered_map<ItemType, ItemTypeData> m_data;
};

#endif