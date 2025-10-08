#include "ButtonComponent.h"

ButtonComponent::ButtonComponent()
    : m_state(ButtonState::Normal)
    , m_enabled(true)
    , m_wasClicked(false)
    , m_previousMousePressed(false)
    , m_selected(false)
    , m_normalColor(sf::Color(70, 70, 70))
    , m_hoverColor(sf::Color(100, 100, 100))
    , m_pressedColor(sf::Color(50, 50, 50))
    , m_disabledColor(sf::Color(40, 40, 40))
    , m_selectedColor(sf::Color(120, 120, 50))
    , m_textColor(sf::Color::White)
{
    m_shape.setSize(sf::Vector2f(200.f, 50.f));
    m_shape.setFillColor(m_normalColor);
    m_shape.setOutlineThickness(2.f);
    m_shape.setOutlineColor(sf::Color::White);
}

bool ButtonComponent::initialize(const sf::Font& font, const std::string& text)
{
    m_text = std::make_unique<sf::Text>(font);
    m_text->setString(text);
    m_text->setCharacterSize(24);
    m_text->setFillColor(m_textColor);

    updateTextPosition();
    return true;
}

/// <summary>
/// CALLBACK EXECUTION FLOW :
// This function detects when the button is clicked and executes
// the stored callback function(m_callback).
// Example flow for "Start Game" button :
    // 1. Game.cpp calls : button.update(mousePos, mousePressed)
    // 2. Button detects : mouse was pressed AND released over button
    // 3. Button executes : m_callback() < --This is the lambda we stored!
    // 4. Lambda runs : [this]() { onStartGame(); }
    // 5. onStartGame() runs : m_stateManager.changeState(GameState::Playing)
    // 6. Game state changes to Playing
    // The button doesn't know WHAT the callback does - it just calls it.
    // This is the "Command Pattern" - the button stores a command and executes it.
/// </summary>
void ButtonComponent::update(const sf::Vector2f& mousePos, bool mousePressed)
{
    m_wasClicked = false;

    if (!m_enabled)
    {
        m_state = ButtonState::Disabled;
        updateColors();
        return;
    }

    // Check if mouse is over button
    bool isMouseOver = m_shape.getGlobalBounds().contains(mousePos);

    if (isMouseOver)
    {
        if (mousePressed)
        {
            m_state = ButtonState::Pressed;
        }
        else if (m_previousMousePressed && !mousePressed)
        {
            // Mouse was released over button - trigger click
            m_wasClicked = true;
            if (m_callback) // This calls whatever lambda was passed
                m_callback();
            m_state = ButtonState::Hovered;
        }
        else
        {
            m_state = ButtonState::Hovered;
        }
    }
    else
    {
        if (m_selected)
            m_state = ButtonState::Selected;
        else
            m_state = ButtonState::Normal;
    }

    // Remember mouse state for next frame
    m_previousMousePressed = mousePressed;
    updateColors();
}

void ButtonComponent::activate()
{
    if (m_enabled && m_callback)
    {
        m_callback();
    }
}

void ButtonComponent::setSelected(bool selected)
{
    m_selected = selected;
    if (selected && m_enabled)
    {
        m_state = ButtonState::Selected;
        updateColors();
    }
}

void ButtonComponent::render(sf::RenderTarget& target) const
{
    target.draw(m_shape);
    if (m_text)
        target.draw(*m_text);
}

void ButtonComponent::setPosition(const sf::Vector2f& pos)
{
    m_shape.setPosition(pos);
    updateTextPosition();
}

void ButtonComponent::setSize(const sf::Vector2f& size)
{
    m_shape.setSize(size);
    updateTextPosition();
}

void ButtonComponent::setText(const std::string& text)
{
    if (m_text)
    {
        m_text->setString(text);
        updateTextPosition();
    }
}

/// <summary>
/// Stores the callback
/// In reference to a function later
/// E.g; m_mainMenu.addButton("Start Game", [this]() { onStartGame(); });
///                                         ^ [capture "this"] (no params) { this function }
/// </summary>
void ButtonComponent::setCallback(std::function<void()> callback)
{
    m_callback = callback;
}

void ButtonComponent::setEnabled(bool enabled)
{
    m_enabled = enabled;
    if (!enabled)
        m_state = ButtonState::Disabled;
    updateColors();
}

void ButtonComponent::setTextColor(const sf::Color& color)
{
    m_textColor = color;
    if (m_text)
        m_text->setFillColor(color);
}

void ButtonComponent::setOutlineColor(const sf::Color& color)
{
    m_shape.setOutlineColor(color);
}

void ButtonComponent::setOutlineThickness(float thickness)
{
    m_shape.setOutlineThickness(thickness);
    updateTextPosition();
}

void ButtonComponent::updateTextPosition()
{
    if (!m_text)
        return;

    sf::FloatRect textBounds = m_text->getLocalBounds();
    sf::FloatRect shapeBounds = m_shape.getLocalBounds();

    float centerX = m_shape.getPosition().x + shapeBounds.size.x / 2.f - textBounds.size.x / 2.f - textBounds.position.x;
    float centerY = m_shape.getPosition().y + shapeBounds.size.y / 2.f - textBounds.size.y / 2.f - textBounds.position.y;

    m_text->setPosition(sf::Vector2f(centerX, centerY));
}

void ButtonComponent::updateColors()
{
    switch (m_state)
    {
    case ButtonState::Normal:
        m_shape.setFillColor(m_normalColor);
        m_shape.setOutlineColor(sf::Color::White);
        m_shape.setOutlineThickness(2.f);
        break;
    case ButtonState::Hovered:
        m_shape.setFillColor(m_hoverColor);
        m_shape.setOutlineColor(sf::Color::White);
        m_shape.setOutlineThickness(2.f);
        break;
    case ButtonState::Pressed:
        m_shape.setFillColor(m_pressedColor);
        m_shape.setOutlineColor(sf::Color::White);
        m_shape.setOutlineThickness(2.f);
        break;
    case ButtonState::Disabled:
        m_shape.setFillColor(m_disabledColor);
        m_shape.setOutlineColor(sf::Color(100, 100, 100));
        m_shape.setOutlineThickness(2.f);
        break;
    case ButtonState::Selected:
        m_shape.setFillColor(m_selectedColor);
        m_shape.setOutlineColor(sf::Color::Yellow);  // Bright outline for selected
        m_shape.setOutlineThickness(4.f);
    }
}