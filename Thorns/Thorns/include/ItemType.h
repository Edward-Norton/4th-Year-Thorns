#ifndef ITEM_TYPE_H
#define ITEM_TYPE_H

#include <SFML/Graphics.hpp>
#include <string>
#include <unordered_map>

enum class ItemType
{
    
    Food,
    Water,
    FirstAid,    
    Bandage,     

    
    Knife,
    Axe,
    Gun,

    COUNT
};

enum class ItemUseCategory
{
    Consume,   
    Equip,     
};

struct ItemTypeData
{
    ItemType     itemType;
    ItemUseCategory useCategory = ItemUseCategory::Consume;
    std::string  name;               
    std::string  atlasKey;           
    sf::IntRect  atlasRect;          
    sf::Vector2f spriteSize;         
    float        statRestoreAmount;  
    float        damage;             
    float        pickupRadius;       
};

class ItemTypeRegistry
{
public:
    ItemTypeRegistry();
    ~ItemTypeRegistry() = default;

    
    bool loadDefinitions(const std::string& definitionsPath);

    
    const ItemTypeData* get(ItemType type) const;

    
    bool has(ItemType type) const;

private:

    void registerDefaults();

    void registerType(ItemType type, const ItemTypeData& data);

    static bool keyToType(const std::string& key, ItemType& outType);

    std::unordered_map<ItemType, ItemTypeData> m_data;
};

#endif