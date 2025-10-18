#ifndef INVENTORY_HPP
#define INVENTORY_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include "IRenderable.h"
#include "IUpdatable.h"
#include "SpriteComponent.h"

// Only FW, not modufying data like in settings (pn)
class InputController;

// ========== ITEM STRUCTURE ==========
struct Item
{
    std::string name;
    std::string texturePath;
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

    // ========== Inventory Management ==========
    bool addItem(const std::string& itemName, const std::string& texturePath, int quantity = 1);
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
};

#endif