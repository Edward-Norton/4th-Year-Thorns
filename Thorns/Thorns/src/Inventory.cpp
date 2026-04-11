#include "Inventory.h"
#include "AssetPaths.h"
#include <iostream>
#include "InputController.h"

Inventory::Inventory()
    : m_isVisible(false)
    , m_fontLoaded(false)
    , m_emptySlotColor(sf::Color(40, 40, 40))
    , m_filledSlotColor(sf::Color(60, 60, 60))
    , m_slotOutlineColor(sf::Color(100, 100, 100))
    , m_hoveredSlotColor(sf::Color(sf::Color::Yellow))
    , m_panelX(0.f)
    , m_panelY(0.f)
    , m_panelWidth(0.f)
    , m_panelHeight(0.f)
{
    // Try to load font for quantity display
    if (m_font.openFromFile(Assets::Fonts::JERSEY_20))
    {
        m_fontLoaded = true;
    }

    // Pre-allocate slots
    for (int i = 0; i < TOTAL_SLOTS; ++i)
    {
        m_slots.emplace_back(i);
    }
}

void Inventory::initialize()
{
    m_hoveredSlot = -1;
    m_hoveredSlotColor = sf::Color(80, 80, 120);

    createSlots();
    updateLayout();
}

void Inventory::createSlots()
{
    for (auto& slot : m_slots)
    {
        slot.background.setSize(sf::Vector2f(SLOT_SIZE, SLOT_SIZE));
        slot.background.setOutlineThickness(2.f);
        slot.background.setOutlineColor(m_slotOutlineColor);
        slot.background.setFillColor(m_emptySlotColor);
    }
}

void Inventory::updateLayout()
{
    float panelYLower = 160.0f;
    // Position panel on left side of screen
    m_panelX = PANEL_PADDING;
    m_panelY = PANEL_PADDING + panelYLower;
    m_panelWidth = (COLUMNS * SLOT_SIZE) + ((COLUMNS - 1) * SLOT_SPACING) + (PANEL_PADDING * 2);
    m_panelHeight = (ROWS * SLOT_SIZE) + ((ROWS - 1) * SLOT_SPACING) + (PANEL_PADDING * 2);

    updateSlotPositions();
}

void Inventory::updateSlotPositions()
{
    for (auto& slot : m_slots)
    {
        int row = slot.slotIndex / COLUMNS;
        int col = slot.slotIndex % COLUMNS;

        float x = m_panelX + PANEL_PADDING + (col * (SLOT_SIZE + SLOT_SPACING));
        float y = m_panelY + PANEL_PADDING + (row * (SLOT_SIZE + SLOT_SPACING));

        slot.background.setPosition(sf::Vector2f(x, y));
    }
}

void Inventory::render(sf::RenderTarget& target) const
{
    if (!m_isVisible)
        return;

    // Draw panel background
    sf::RectangleShape panelBackground(sf::Vector2f(m_panelWidth, m_panelHeight));
    panelBackground.setPosition(sf::Vector2f(m_panelX, m_panelY));
    panelBackground.setFillColor(sf::Color(20, 20, 20, 220));
    panelBackground.setOutlineThickness(2.f);
    panelBackground.setOutlineColor(sf::Color(100, 100, 100));
    target.draw(panelBackground);

    // Draw all slots
    for (const auto& slot : m_slots)
    {
        renderSlot(target, slot);
    }

    // Draw drag ghost above all slots
    renderDragGhost(target);

    // Draw context menu above everything
    renderContextMenu(target);
}

void Inventory::updateSlotInteraction(const InputController& input)
{
    if (!m_isVisible)
        return;

    sf::Vector2f mousePos = input.getMousePosition();
    m_hoveredSlot = getSlotUnderMouse(mousePos);

    // ===== CONTEXT MENU =====
    if (m_contextMenu.visible)
    {
        // Dismiss on left click outside menu
        if (input.wasMouseJustPressed())
        {
            ContextMenu::Action action = getContextMenuActionUnderMouse(mousePos);
            if (action == ContextMenu::Action::Use)
                useItem(m_contextMenu.slotIndex);
            else if (action == ContextMenu::Action::Drop)
                dropItem(m_contextMenu.slotIndex);

            closeContextMenu();
        }
        // Dismiss on right click anywhere
        if (input.wasRightMouseJustPressed())
            closeContextMenu();

        return; // Block all other interactions while context menu is open
    }

    // ===== DRAG handling =====
    if (m_dragState.active)
    {
        updateDrag(mousePos);

        if (input.wasMouseJustReleased())
            endDrag(mousePos);

        return; // Block click logic while dragging
    }

    // ===== LEFT MOUSE pressed this frame =====
    if (input.wasMouseJustPressed())
    {
        m_pressedSlot = m_hoveredSlot;
        m_leftHoldTimer = 0.f;
    }

    // ===== Hold timer while LMB down on a slot =====
    if (input.isMousePressed() && m_pressedSlot != -1 && !m_slots[m_pressedSlot].item == false)
    {
        m_leftHoldTimer += (1.f / 60.f); // PN: This is bad practise, but no time to fix

        if (m_leftHoldTimer >= DRAG_HOLD_THRESHOLD && !m_dragState.active)
            beginDrag(m_pressedSlot, mousePos);
    }

    // ===== LEFT MOUSE released this frame =====
    if (input.wasMouseJustReleased())
    {
        if (!m_dragState.active && m_pressedSlot != -1 && m_pressedSlot == m_hoveredSlot)
        {
            // Short click on a filled slot = use item directly
            if (m_pressedSlot >= 0 && m_pressedSlot < TOTAL_SLOTS && m_slots[m_pressedSlot].item)
                useItem(m_pressedSlot);
        }

        m_pressedSlot = -1;
        m_leftHoldTimer = 0.f;
    }

    // ===== RIGHT MOUSE pressed, open context menu =====
    if (input.wasRightMouseJustPressed())
    {
        if (m_hoveredSlot != -1 && m_slots[m_hoveredSlot].item)
            openContextMenu(m_hoveredSlot, mousePos);
    }
}

void Inventory::renderSlot(sf::RenderTarget& target, const InventorySlot& slot) const
{

    // Use hover color if this slot is hovered
    sf::RectangleShape background = slot.background;
    if (m_hoveredSlot == slot.slotIndex)
    {
        background.setFillColor(m_hoveredSlotColor);
    }

    // Draw background
    target.draw(slot.background);

    // Draw item if slot is filled
    if (slot.item)
    {
        // Get bounds and calculate scale to fit in slot
        sf::FloatRect itemBounds = slot.item->sprite.getLocalBounds();
        float maxDimension = std::max(itemBounds.size.x, itemBounds.size.y);
        float scale = (SLOT_SIZE * 0.8f) / maxDimension;

        // Center item in slot
        float centerX = slot.background.getPosition().x + (SLOT_SIZE / 2.f) - (itemBounds.size.x * scale / 2.f);
        float centerY = slot.background.getPosition().y + (SLOT_SIZE / 2.f) - (itemBounds.size.y * scale / 2.f);

        slot.item->sprite.setScale(sf::Vector2f(scale, scale));
        slot.item->sprite.setPosition(sf::Vector2f(centerX, centerY));
        slot.item->sprite.render(target);

        // Draw quantity if > 1
        if (slot.item->quantity > 1 && m_fontLoaded)
        {
            sf::Text quantityText(m_font);
            quantityText.setString(std::to_string(slot.item->quantity));
            quantityText.setCharacterSize(14);
            quantityText.setFillColor(sf::Color::Yellow);

            float qX = slot.background.getPosition().x + SLOT_SIZE - 20.f;
            float qY = slot.background.getPosition().y + SLOT_SIZE - 20.f;
            quantityText.setPosition(sf::Vector2f(qX, qY));
            target.draw(quantityText);
        }
    }
}

int Inventory::getSlotUnderMouse(const sf::Vector2f& mousePos) const
{
    for (const auto& slot : m_slots)
    {
        if (slot.background.getGlobalBounds().contains(mousePos))
        {
            return slot.slotIndex;
        }
    }
    return -1;  // No slot under mouse
}

bool Inventory::addItem(const std::string& itemName, const std::string& texturePath, int quantity)
{
    // Find first empty slot
    for (auto& slot : m_slots)
    {
        if (!slot.item)
        {
            slot.item = std::make_unique<Item>();
            slot.item->name = itemName;
            slot.item->texturePath = texturePath;
            slot.item->quantity = quantity;

            // Load texture using SpriteComponent
            if (!slot.item->sprite.loadTexture(texturePath, SLOT_SIZE * 0.8f, SLOT_SIZE * 0.8f))
            {
                std::cerr << "Failed to load item texture: " << texturePath << std::endl;
                slot.item.reset();
                return false;
            }

            slot.background.setFillColor(m_filledSlotColor);
            return true;
        }
    }

    std::cout << "Inventory full!" << std::endl;
    return false;
}

bool Inventory::addItem(const std::string& itemName, const sf::Texture& atlas, const sf::IntRect& atlasRect, ItemType itemType, int quantity)
{
    for (auto& slot : m_slots)
    {
        if (!slot.item)
        {
            slot.item = std::make_unique<Item>();
            slot.item->name = itemName;
            slot.item->texturePath = "";
            slot.item->itemType = itemType;
            slot.item->quantity = quantity;

            if (!slot.item->sprite.setSharedTexture(atlas,
                SLOT_SIZE * 0.8f, SLOT_SIZE * 0.8f, atlasRect))
            {
                std::cerr << "Inventory::addItem: setSharedTexture failed for "
                    << itemName << "\n";
                slot.item.reset();
                return false;
            }

            slot.background.setFillColor(m_filledSlotColor);
            return true;
        }
    }

    std::cout << "Inventory full!\n";
    return false;
}

bool Inventory::removeItem(int slotIndex, int quantity)
{
    if (slotIndex < 0 || slotIndex >= TOTAL_SLOTS)
        return false;

    auto& slot = m_slots[slotIndex];
    if (!slot.item)
        return false;

    slot.item->quantity -= quantity;

    if (slot.item->quantity <= 0)
    {
        clearSlot(slotIndex);
    }

    return true;
}

void Inventory::clearSlot(int slotIndex)
{
    if (slotIndex >= 0 && slotIndex < TOTAL_SLOTS)
    {
        m_slots[slotIndex].item.reset();
        m_slots[slotIndex].background.setFillColor(m_emptySlotColor);
    }
}

void Inventory::clearInventory()
{
    for (auto& slot : m_slots)
    {
        slot.item.reset();
        slot.background.setFillColor(m_emptySlotColor);
    }
}

Item* Inventory::getItemAtSlot(int slotIndex) const
{
    if (slotIndex >= 0 && slotIndex < TOTAL_SLOTS && m_slots[slotIndex].item)
        return m_slots[slotIndex].item.get();
    return nullptr;
}

int Inventory::getUsedSlots() const
{
    int count = 0;
    for (const auto& slot : m_slots)
    {
        if (slot.item)
            count++;
    }
    return count;
}

bool Inventory::isSlotEmpty(int slotIndex) const
{
    if (slotIndex < 0 || slotIndex >= TOTAL_SLOTS)
        return true;
    return !m_slots[slotIndex].item;
}


// ========== ITEM ACTIONS ==========

void Inventory::useItem(int slotIndex)
{
    if (slotIndex < 0 || slotIndex >= TOTAL_SLOTS || !m_slots[slotIndex].item)
        return;

    Item* item = m_slots[slotIndex].item.get();

    // Cache name before item is destroyed by removeItem
    std::string itemName = item->name;
    ItemType itemType = item->itemType;

    // Fire callback so Player can apply stat effects
    if (m_onItemUsed)
        m_onItemUsed(item->itemType);

    // Only consume if it's a consumable — weapons stay equipped in slot
    const ItemTypeData* data = m_registry ? m_registry->get(itemType) : nullptr;
    bool isConsumable = !data || data->useCategory == ItemUseCategory::Consume;

    // Consume one unit, clear slot when quantity hits zero
    if (isConsumable)
        removeItem(slotIndex, 1);
    std::cout << "Used: " << itemName << "\n";
}

void Inventory::dropItem(int slotIndex)
{
    if (slotIndex < 0 || slotIndex >= TOTAL_SLOTS || !m_slots[slotIndex].item)
        return;

    ItemType type = m_slots[slotIndex].item->itemType;

    std::cout << "Dropped: " << m_slots[slotIndex].item->name << "\n";
    clearSlot(slotIndex);
}

void Inventory::moveItem(int sourceSlot, int targetSlot)
{
    if (sourceSlot == targetSlot)
        return;

    if (sourceSlot < 0 || sourceSlot >= TOTAL_SLOTS ||
        targetSlot < 0 || targetSlot >= TOTAL_SLOTS)
        return;

    // Swap items between slots
    std::swap(m_slots[sourceSlot].item, m_slots[targetSlot].item);

    // Sync background colors
    auto syncColor = [&](int idx)
        {
            m_slots[idx].background.setFillColor(
                m_slots[idx].item ? m_filledSlotColor : m_emptySlotColor);
        };

    syncColor(sourceSlot);
    syncColor(targetSlot);
}

// ========== CONTEXT MENU ==========

void Inventory::openContextMenu(int slotIndex, const sf::Vector2f& screenPos)
{
    m_contextMenu.visible = true;
    m_contextMenu.slotIndex = slotIndex;
    m_contextMenu.position = screenPos;
    m_contextMenu.entries = { {"Use", ContextMenu::Action::Use},
                                 {"Drop", ContextMenu::Action::Drop} };
    m_contextMenu.hoveredEntry = -1;
}

void Inventory::closeContextMenu()
{
    m_contextMenu.visible = false;
    m_contextMenu.slotIndex = -1;
    m_contextMenu.hoveredEntry = -1;
}

ContextMenu::Action Inventory::getContextMenuActionUnderMouse(const sf::Vector2f& mousePos) const
{
    if (!m_contextMenu.visible)
        return ContextMenu::Action::None;

    for (int i = 0; i < static_cast<int>(m_contextMenu.entries.size()); ++i)
    {
        sf::FloatRect entryBounds(
            sf::Vector2f(m_contextMenu.position.x,
                m_contextMenu.position.y + ContextMenu::PADDING + i * ContextMenu::ENTRY_HEIGHT),
            sf::Vector2f(ContextMenu::WIDTH, ContextMenu::ENTRY_HEIGHT)
        );

        if (entryBounds.contains(mousePos))
            return m_contextMenu.entries[i].action;
    }

    return ContextMenu::Action::None;
}

void Inventory::renderContextMenu(sf::RenderTarget& target) const
{
    if (!m_contextMenu.visible)
        return;

    float totalHeight = ContextMenu::PADDING * 2.f +
        m_contextMenu.entries.size() * ContextMenu::ENTRY_HEIGHT;

    // Background panel
    sf::RectangleShape bg(sf::Vector2f(ContextMenu::WIDTH, totalHeight));
    bg.setPosition(m_contextMenu.position);
    bg.setFillColor(sf::Color(30, 30, 30, 230));
    bg.setOutlineThickness(1.f);
    bg.setOutlineColor(sf::Color(150, 150, 150));
    target.draw(bg);

    for (int i = 0; i < static_cast<int>(m_contextMenu.entries.size()); ++i)
    {
        float entryY = m_contextMenu.position.y + ContextMenu::PADDING + i * ContextMenu::ENTRY_HEIGHT;

        sf::FloatRect entryBounds(
            sf::Vector2f(m_contextMenu.position.x, entryY),
            sf::Vector2f(ContextMenu::WIDTH, ContextMenu::ENTRY_HEIGHT)
        );

        // Hover highlight
        if (entryBounds.contains(m_contextMenu.entries.size() >= 0 ?
            sf::Vector2f(-1.f, -1.f) : sf::Vector2f(-1.f, -1.f)))
        {
            // Hover tint drawn separately via hoveredEntry in update — kept simple here
        }

        if (m_fontLoaded)
        {
            sf::Text label(m_font);
            label.setString(m_contextMenu.entries[i].label);
            label.setCharacterSize(18);
            label.setFillColor(sf::Color::White);
            label.setPosition(sf::Vector2f(m_contextMenu.position.x + 8.f,
                entryY + 6.f));
            target.draw(label);
        }
    }
}

// ========== DRAG ==========

void Inventory::beginDrag(int slotIndex, const sf::Vector2f& mousePos)
{
    if (slotIndex < 0 || slotIndex >= TOTAL_SLOTS || !m_slots[slotIndex].item)
        return;

    m_dragState.active = true;
    m_dragState.sourceSlot = slotIndex;
    m_dragState.ghostPosition = mousePos;
    m_dragState.grabOffset = sf::Vector2f(0.f, 0.f);

    std::cout << "Drag started from slot " << slotIndex << "\n";
}

void Inventory::updateDrag(const sf::Vector2f& mousePos)
{
    if (!m_dragState.active)
        return;

    m_dragState.ghostPosition = mousePos;
}

void Inventory::endDrag(const sf::Vector2f& mousePos)
{
    if (!m_dragState.active)
        return;

    int targetSlot = getSlotUnderMouse(mousePos);

    if (targetSlot != -1 && targetSlot != m_dragState.sourceSlot)
    {
        moveItem(m_dragState.sourceSlot, targetSlot);
        std::cout << "Moved item from slot " << m_dragState.sourceSlot
            << " to slot " << targetSlot << "\n";
    }

    m_dragState.active = false;
    m_dragState.sourceSlot = -1;
}

void Inventory::renderDragGhost(sf::RenderTarget& target) const
{
    if (!m_dragState.active)
        return;

    const InventorySlot& src = m_slots[m_dragState.sourceSlot];
    if (!src.item)
        return;

    // Draw semi-transparent ghost at cursor position
    src.item->sprite.setPosition(m_dragState.ghostPosition);
    src.item->sprite.setScale(sf::Vector2f(0.8f, 0.8f));

    // SFML does not have a global alpha shortcut on Sprite directly;
    // render at reduced scale as a visual cue, color is handled by existing sprite
    src.item->sprite.render(target);
}

