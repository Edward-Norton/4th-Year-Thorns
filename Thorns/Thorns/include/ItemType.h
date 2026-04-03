#ifndef ITEM_TYPE_H
#define ITEM_TYPE_H

#include <SFML/Graphics.hpp>
#include <string>
#include <unordered_map>

// ========== ITEM TYPES ==========
enum class ItemType
{
    Food,
    Water,
    COUNT
};

// ========== FLYWEIGHT DATA ==========
// All of the items are to use this format, only need it once per object
struct ItemTypeData
{
    std::string  name;               // Display name shown in inventory
    sf::IntRect  atlasRect;          // Region inside the shared items atlas
    sf::Vector2f spriteSize;         // World-space render size in pixels
    float        statRestoreAmount;  // Units of the stat restored on use
    float        pickupRadius;       // Pixel radius at which player triggers collection
};

// ========== ITEM TYPE REGISTRY ==========
class ItemTypeRegistry
{
public:
    ItemTypeRegistry();
    ~ItemTypeRegistry() = default;

    // Retrieve shared data for a type. Returns nullptr if type not registered.
    const ItemTypeData* get(ItemType type) const;

    // Returns true if type has registered data.
    bool has(ItemType type) const;

private:
    void loadDefaults();
    void registerType(ItemType type, const ItemTypeData& data);

    std::unordered_map<ItemType, ItemTypeData> m_data;
};

#endif