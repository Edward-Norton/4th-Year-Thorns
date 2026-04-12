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
    
    if (!m_font.openFromFile(fontPath))
    {
        std::cerr << "Menu: Failed to load font: " << fontPath << '\n';
        return false;
    }

    
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
        updateLayout();  
    }
}

void Menu::setPosition(const sf::Vector2f& pos)
{
    m_position = pos;
    updateLayout();  
}

void Menu::addButton(const std::string& text, std::function<void()> callback)
{
    
    auto button = std::make_unique<ButtonComponent>();
    button->initialize(m_font, text);
    button->setSize(sf::Vector2f(250.f, 50.f));
    button->setCallback(callback);

    
    m_buttons.push_back(std::move(button));

    
    updateLayout();
}

void Menu::clearButtons()
{
    m_buttons.clear();
}

void Menu::update(const InputController& input)
{
    
    if (!m_visible || m_buttons.empty())
        return;

    
    InputDevice device = input.getActiveDevice();

    
    if (device == InputDevice::Keyboard || device == InputDevice::Gamepad)
    {
        handleKeyboardGamepadInput(input);
    }

    
    handleMouseInput(input);

    
    updateButtonVisuals(input);
}

void Menu::handleKeyboardGamepadInput(const InputController& input)
{
    
    if (input.wasJustPressed(InputAction::MoveUp))
    {
        m_selectedIndex--;
        if (m_selectedIndex < 0)
            m_selectedIndex = static_cast<int>(m_buttons.size()) - 1;  
    }

    
    if (input.wasJustPressed(InputAction::MoveDown))
    {
        m_selectedIndex++;
        if (m_selectedIndex >= static_cast<int>(m_buttons.size()))
            m_selectedIndex = 0;  
    }

    
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

    
    for (size_t i = 0; i < m_buttons.size(); ++i)
    {
        if (m_buttons[i]->getBounds().contains(mousePos))
        {
            m_selectedIndex = static_cast<int>(i);
            break;
        }
    }

    
    bool mousePressed = input.isMousePressed();
    for (auto& button : m_buttons)
    {
        button->update(mousePos, mousePressed);
    }
}

void Menu::updateButtonVisuals(const InputController& input)
{
    
    for (size_t i = 0; i < m_buttons.size(); ++i)
    {
        bool isSelected = (static_cast<int>(i) == m_selectedIndex);
        m_buttons[i]->setSelected(isSelected);
    }
}

void Menu::render(sf::RenderTarget& target) const
{
    
    if (!m_visible)
        return;

    
    if (m_titleText && !m_titleText->getString().isEmpty())
    {
        target.draw(*m_titleText);
    }

    
    for (const auto& button : m_buttons)
    {
        button->render(target);
    }
}

void Menu::updateLayout()
{
    

    float currentY = m_position.y;

    
    if (m_titleText && !m_titleText->getString().isEmpty())
    {
        sf::FloatRect bounds = m_titleText->getLocalBounds();

        
        m_titleText->setPosition(sf::Vector2f(
            m_position.x + 125.f - bounds.size.x / 2.f,  
            currentY - 100.f                              
        ));
    }

    
    for (auto& button : m_buttons)
    {
        button->setPosition(sf::Vector2f(m_position.x, currentY));
        currentY += m_buttonSpacing;  
    }
}