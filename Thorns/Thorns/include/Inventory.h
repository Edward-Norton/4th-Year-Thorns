#ifndef INVENTORY_HPP
#define INVENTORY_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include "IRenderable.h"
#include "IUpdatable.h"
#include "SpriteComponent.h"
#include "ItemType.h"
#include <functional>

class InputController;

struct Item
{
    std::string name;
    std::string texturePath;
    ItemType itemType;
    SpriteComponent sprite;
    int quantity = 1;
};

struct InventorySlot
{
    std::unique_ptr<Item> item;
    sf::RectangleShape background;
    int slotIndex;

    InventorySlot(int index) : slotIndex(index) {}
};

struct ContextMenu
{
    
    enum class Action { None, Use, Drop };

    bool        visible = false;
    int         slotIndex = -1;       
    sf::Vector2f position;              

    
    struct Entry { std::string label; Action action; };
    std::vector<Entry> entries;

    int hoveredEntry = -1;

    
    static constexpr float WIDTH = 120.f;
    static constexpr float ENTRY_HEIGHT = 32.f;
    static constexpr float PADDING = 8.f;
};

struct DragState
{
    bool    active = false;
    int     sourceSlot = -1;          
    sf::Vector2f grabOffset;            
    sf::Vector2f ghostPosition;         
};

class Inventory : public IRenderable
{
public:
    Inventory();
    ~Inventory() = default;

    
    void initialize();

    
    void render(sf::RenderTarget& target) const override;

    void updateSlotInteraction(const InputController& input);

    void setRegistry(const ItemTypeRegistry* registry) { m_registry = registry; }

    
    void setOnItemUsed(std::function<void(ItemType)> callback) { m_onItemUsed = callback; }
    
    void setOnItemDropped(std::function<void(ItemType, sf::Vector2f)> callback) { m_onItemDropped = callback; }

    
    bool addItem(const std::string& itemName, const std::string& texturePath, int quantity = 1);
    bool addItem(const std::string& itemName, const sf::Texture& atlas, const sf::IntRect& atlasRect, ItemType itemType, int quantity = 1);

    bool removeItem(int slotIndex, int quantity = 1);
    void clearSlot(int slotIndex);
    void clearInventory();

    
    Item* getItemAtSlot(int slotIndex) const;
    int getTotalSlots() const { return m_slots.size(); }
    int getUsedSlots() const;
    bool isSlotEmpty(int slotIndex) const;

    
    void setVisible(bool visible) { m_isVisible = visible; }
    bool isVisible() const { return m_isVisible; }
    void toggle() { m_isVisible = !m_isVisible; }

    
    void updateLayout();

private:
    void createSlots();
    void updateSlotPositions();
    void renderSlot(sf::RenderTarget& target, const InventorySlot& slot) const;
    int getSlotUnderMouse(const sf::Vector2f& mousePos) const;

    
    void useItem(int slotIndex);
    void dropItem(int slotIndex);
    void moveItem(int sourceSlot, int targetSlot);

    
    void openContextMenu(int slotIndex, const sf::Vector2f& screenPos);
    void closeContextMenu();
    
    ContextMenu::Action getContextMenuActionUnderMouse(const sf::Vector2f& mousePos) const;
    void renderContextMenu(sf::RenderTarget& target) const;

    
    void beginDrag(int slotIndex, const sf::Vector2f& mousePos);
    void updateDrag(const sf::Vector2f& mousePos);
    void endDrag(const sf::Vector2f& mousePos);
    void renderDragGhost(sf::RenderTarget& target) const;

    
    std::vector<InventorySlot> m_slots;
    static constexpr int COLUMNS = 2;
    static constexpr int ROWS = 5;
    static constexpr int TOTAL_SLOTS = COLUMNS * ROWS;  

    
    static constexpr float SLOT_SIZE = 60.f;
    static constexpr float SLOT_SPACING = 10.f;
    static constexpr float PANEL_PADDING = 20.f;

    float m_panelX;
    float m_panelY;
    float m_panelWidth;
    float m_panelHeight;

    
    bool m_isVisible;
    sf::Font m_font;  
    bool m_fontLoaded;

    
    sf::Color m_emptySlotColor;
    sf::Color m_filledSlotColor;
    sf::Color m_slotOutlineColor;
    sf::Color m_hoveredSlotColor;

    
    int m_hoveredSlot; 

    
    ContextMenu m_contextMenu;
    DragState   m_dragState;

    
    static constexpr float DRAG_HOLD_THRESHOLD = 0.15f;
    float        m_leftHoldTimer = 0.f;   
    int          m_pressedSlot = -1;    

    
    std::function<void(ItemType)> m_onItemUsed;
    std::function<void(ItemType, sf::Vector2f)> m_onItemDropped;

    
    const ItemTypeRegistry* m_registry = nullptr;
};

#endif