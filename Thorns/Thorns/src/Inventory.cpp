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
    // Position panel on left side of screen
    m_panelX = PANEL_PADDING;
    m_panelY = PANEL_PADDING;
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
}

void Inventory::updateSlotInteraction(const InputController& input)
{
    if (!m_isVisible)
        return;

    sf::Vector2f mousePos = input.getMousePosition();
    m_hoveredSlot = getSlotUnderMouse(mousePos);

    // Handle click on slot
    if (m_hoveredSlot != -1 && input.wasMouseJustPressed())
    {
        std::cout << "Clicked slot " << m_hoveredSlot << std::endl;
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