#ifndef ITEM_TYPE_H
#define ITEM_TYPE_H

#include <SFML/Graphics.hpp>
#include <string>
#include <unordered_map>

// ========== ITEM TYPES ==========
// Add new consumable types here only. No other files need changing for new types.
enum class ItemType
{
    Food,
    Water,
    COUNT
};

// ========== FLYWEIGHT DATA ==========
// Shared, read-only data per item type.
// One entry exists for the entire program lifetime.
// All WorldItem instances point into this table rather than owning their own copy.
// Pattern: Flyweight (Game Programming Patterns ch.3)
struct ItemTypeData
{
    std::string  name;               // Display name shown in inventory
    sf::IntRect  atlasRect;          // Region inside the shared items atlas
    sf::Vector2f spriteSize;         // World-space render size in pixels (recommended: 32x32)
    float        statRestoreAmount;  // Units of the relevant stat restored on use
    float        pickupRadius;       // Pixel radius at which player triggers collection
};

// ========== ITEM TYPE REGISTRY ==========
// Loads and stores one ItemTypeData per ItemType.
// Constructed once in Game, passed by const ref wherever item data is needed.
// Mirrors POIConfigRegistry in purpose and structure.
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