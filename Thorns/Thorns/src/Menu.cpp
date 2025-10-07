#include "Menu.h"
#include <iostream>

Menu::Menu()
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

void Menu::update(const sf::Vector2f& mousePos, bool mousePressed)
{
    // Skip if menu is hidden
    if (!m_visible)
        return;

    // Update all buttons (hover detection, click handling)
    for (auto& button : m_buttons)
    {
        button->update(mousePos, mousePressed);
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
     * Layout Algorithm:
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