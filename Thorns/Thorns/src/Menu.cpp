#include "Menu.h"
#include "InputController.h"
#include <iostream>

Menu::Menu()
    : m_position(800.f, 300.f)
    , m_buttonSpacing(60.f)
    , m_visible(true)
    , m_selectedIndex(0)
{
}

bool Menu::initialize(const std::string& fontPath)
{
    // Load font from file
    if (!m_font.openFromFile(fontPath))
    {
        std::cerr << "Menu: Failed to load font: " << fontPath << '\n';
        return false;
    }

    // Create title text object
    m_titleText = std::make_unique<sf::Text>(m_font);
    m_titleText->setCharacterSize(50);
    m_titleText->setFillColor(sf::Color::White);

    return true;
}

void Menu::setTitle(const std::string& title)
{
    if (m_titleText)
    {
        m_titleText->setString(title);
        updateLayout();  // Reposition elements when title changes
    }
}

void Menu::setPosition(const sf::Vector2f& pos)
{
    m_position = pos;
    updateLayout();  // Reposition all buttons
}

void Menu::addButton(const std::string& text, std::function<void()> callback)
{
    // Create new button
    auto button = std::make_unique<ButtonComponent>();
    button->initialize(m_font, text);
    button->setSize(sf::Vector2f(250.f, 50.f));
    button->setCallback(callback);

    // Add to button list
    m_buttons.push_back(std::move(button));

    // Update all button positions
    updateLayout();
}

void Menu::clearButtons()
{
    m_buttons.clear();
}

void Menu::update(const InputController& input)
{
    // Skip if menu is hidden or has no buttons
    if (!m_visible || m_buttons.empty())
        return;

    // Get active device for context
    InputDevice device = input.getActiveDevice();

    // Handle keyboard/gamepad navigation (directional + confirm)
    if (device == InputDevice::Keyboard || device == InputDevice::Gamepad)
    {
        handleKeyboardGamepadInput(input);
    }

    // Mouse always works
    handleMouseInput(input);

    // Update button visual states (selection highlights)
    updateButtonVisuals(input);
}

void Menu::handleKeyboardGamepadInput(const InputController& input)
{
    // Navigate up (W key or gamepad stick up)
    if (input.wasJustPressed(InputAction::MoveUp))
    {
        m_selectedIndex--;
        if (m_selectedIndex < 0)
            m_selectedIndex = static_cast<int>(m_buttons.size()) - 1;  // Wrap to bottom
    }

    // Navigate down (S key or gamepad stick down)
    if (input.wasJustPressed(InputAction::MoveDown))
    {
        m_selectedIndex++;
        if (m_selectedIndex >= static_cast<int>(m_buttons.size()))
            m_selectedIndex = 0;  // Wrap to top
    }

    // Activate selected button (Enter key or A button on gamepad)
    if (input.wasJustPressed(InputAction::Confirm))
    {
        if (m_selectedIndex >= 0 && m_selectedIndex < static_cast<int>(m_buttons.size()))
        {
            m_buttons[m_selectedIndex]->activate();
        }
    }
}

void Menu::handleMouseInput(const InputController& input)
{
    sf::Vector2f mousePos = input.getMousePosition();

    // Check if mouse is over any button (updates selection)
    for (size_t i = 0; i < m_buttons.size(); ++i)
    {
        if (m_buttons[i]->getBounds().contains(mousePos))
        {
            m_selectedIndex = static_cast<int>(i);
            break;
        }
    }

    // Update buttons with mouse state (for click detection)
    bool mousePressed = input.isMousePressed();
    for (auto& button : m_buttons)
    {
        button->update(mousePos, mousePressed);
    }
}

void Menu::updateButtonVisuals(const InputController& input)
{
    // Update selection state for all buttons
    for (size_t i = 0; i < m_buttons.size(); ++i)
    {
        bool isSelected = (static_cast<int>(i) == m_selectedIndex);
        m_buttons[i]->setSelected(isSelected);
    }
}

void Menu::render(sf::RenderTarget& target) const
{
    // Skip if menu is hidden
    if (!m_visible)
        return;

    // Draw title if it exists
    if (m_titleText && !m_titleText->getString().isEmpty())
    {
        target.draw(*m_titleText);
    }

    // Draw all buttons
    for (const auto& button : m_buttons)
    {
        button->render(target);
    }
}

void Menu::updateLayout()
{
    /*
     * Layout:
     * 1. Position title above buttons (centered)
     * 2. Stack buttons vertically starting at m_position
     * 3. Apply spacing between buttons
     */

    float currentY = m_position.y;

    // Position title (centered above buttons)
    if (m_titleText && !m_titleText->getString().isEmpty())
    {
        sf::FloatRect bounds = m_titleText->getLocalBounds();

        // Center horizontally above buttons
        m_titleText->setPosition(sf::Vector2f(
            m_position.x + 125.f - bounds.size.x / 2.f,  // 125 = half button width
            currentY - 100.f                              // 100px above buttons
        ));
    }

    // Position buttons vertically
    for (auto& button : m_buttons)
    {
        button->setPosition(sf::Vector2f(m_position.x, currentY));
        currentY += m_buttonSpacing;  // Move down for next button
    }
}