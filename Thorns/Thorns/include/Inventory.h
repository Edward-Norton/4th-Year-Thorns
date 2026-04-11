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

// Only FW, not modufying data like in settings (pn)
class InputController;

// ========== ITEM STRUCTURE ==========
struct Item
{
    std::string name;
    std::string texturePath;
    ItemType itemType;
    SpriteComponent sprite;
    int quantity = 1;
};

// ========== INVENTORY SLOT ==========
struct InventorySlot
{
    std::unique_ptr<Item> item;
    sf::RectangleShape background;
    int slotIndex;

    InventorySlot(int index) : slotIndex(index) {}
};

// ========== CONTEXT MENU ==========
struct ContextMenu
{
    // Actions the menu can request. Inventory fires the matching callback.
    enum class Action { None, Use, Drop };

    bool        visible = false;
    int         slotIndex = -1;       // Slot the menu was opened for
    sf::Vector2f position;              // Top-left anchor in screen space

    // Visual entries: label text and the action it maps to
    struct Entry { std::string label; Action action; };
    std::vector<Entry> entries;

    int hoveredEntry = -1;

    // Layout constants
    static constexpr float WIDTH = 120.f;
    static constexpr float ENTRY_HEIGHT = 32.f;
    static constexpr float PADDING = 8.f;
};

// ========== DRAG STATE ==========
// Tracks a slot being dragged. Ghost sprite follows the cursor.
struct DragState
{
    bool    active = false;
    int     sourceSlot = -1;          // Which slot is being dragged
    sf::Vector2f grabOffset;            // Offset from item center to cursor at grab time
    sf::Vector2f ghostPosition;         // Current ghost sprite position
};

class Inventory : public IRenderable
{
public:
    Inventory();
    ~Inventory() = default;

    // ========== Initialization ==========
    void initialize();

    // ========== IRenderable ==========
    void render(sf::RenderTarget& target) const override;


    void updateSlotInteraction(const InputController& input);

    void setRegistry(const ItemTypeRegistry* registry) { m_registry = registry; }


    // ========== Item Use Callback ==========
    void setOnItemUsed(std::function<void(ItemType)> callback) { m_onItemUsed = callback; }
    // When dropped resapwn it
    void setOnItemDropped(std::function<void(ItemType, sf::Vector2f)> callback) { m_onItemDropped = callback; }


    // ========== Inventory Management ==========
    bool addItem(const std::string& itemName, const std::string& texturePath, int quantity = 1);
    bool addItem(const std::string& itemName, const sf::Texture& atlas, const sf::IntRect& atlasRect, ItemType itemType, int quantity = 1);

    bool removeItem(int slotIndex, int quantity = 1);
    void clearSlot(int slotIndex);
    void clearInventory();

    // ========== Queries ==========
    Item* getItemAtSlot(int slotIndex) const;
    int getTotalSlots() const { return m_slots.size(); }
    int getUsedSlots() const;
    bool isSlotEmpty(int slotIndex) const;

    // ========== Visibility ==========
    void setVisible(bool visible) { m_isVisible = visible; }
    bool isVisible() const { return m_isVisible; }
    void toggle() { m_isVisible = !m_isVisible; }

    // ========== Layout ==========
    void updateLayout();

private:
    void createSlots();
    void updateSlotPositions();
    void renderSlot(sf::RenderTarget& target, const InventorySlot& slot) const;
    int getSlotUnderMouse(const sf::Vector2f& mousePos) const;


    // ========== Item Actions ==========
    void useItem(int slotIndex);
    void dropItem(int slotIndex);
    void moveItem(int sourceSlot, int targetSlot);

    // ========== Context Menu Helpers ==========
    void openContextMenu(int slotIndex, const sf::Vector2f& screenPos);
    void closeContextMenu();
    // Returns the ContextMenu::Action under mousePos, or Action::None.
    ContextMenu::Action getContextMenuActionUnderMouse(const sf::Vector2f& mousePos) const;
    void renderContextMenu(sf::RenderTarget& target) const;

    // ========== Drag Helpers ==========
    void beginDrag(int slotIndex, const sf::Vector2f& mousePos);
    void updateDrag(const sf::Vector2f& mousePos);
    void endDrag(const sf::Vector2f& mousePos);
    void renderDragGhost(sf::RenderTarget& target) const;

    // ========== Slots ==========
    std::vector<InventorySlot> m_slots;
    static constexpr int COLUMNS = 2;
    static constexpr int ROWS = 5;
    static constexpr int TOTAL_SLOTS = COLUMNS * ROWS;  // 10 slots

    // ========== Layout ==========
    static constexpr float SLOT_SIZE = 60.f;
    static constexpr float SLOT_SPACING = 10.f;
    static constexpr float PANEL_PADDING = 20.f;

    float m_panelX;
    float m_panelY;
    float m_panelWidth;
    float m_panelHeight;

    // ========== State ==========
    bool m_isVisible;
    sf::Font m_font;  // For displaying item quantities
    bool m_fontLoaded;

    // ========== Colors ==========
    sf::Color m_emptySlotColor;
    sf::Color m_filledSlotColor;
    sf::Color m_slotOutlineColor;
    sf::Color m_hoveredSlotColor;

    // ========== Input ==========
    int m_hoveredSlot; //Track which slot is hovered

    // ========== Interaction State ==========
    ContextMenu m_contextMenu;
    DragState   m_dragState;

    // Drag hold threshold in seconds before drag activates, just to prevent accidental drags
    static constexpr float DRAG_HOLD_THRESHOLD = 0.15f;
    float        m_leftHoldTimer = 0.f;   // Accumulates while LMB held over a slot
    int          m_pressedSlot = -1;    // Slot where LMB went down this interaction

    // ========== Callback ==========
    std::function<void(ItemType)> m_onItemUsed;
    std::function<void(ItemType, sf::Vector2f)> m_onItemDropped;

    // Item type
    const ItemTypeRegistry* m_registry = nullptr;
};

#endif