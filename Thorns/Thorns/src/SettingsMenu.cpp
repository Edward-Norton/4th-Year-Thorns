#include "SettingsMenu.h"
#include <iostream>

SettingsMenu::SettingsMenu()
    : m_inputController(nullptr)
    , m_visible(false)
    , m_waitingForKey(false)
    , m_actionToRebind(InputAction::MoveUp)
    , m_backCallback(nullptr)
{
}

bool SettingsMenu::initialize(const std::string& fontPath, InputController* inputController)
{
    // Load font
    if (!m_font.openFromFile(fontPath))
    {
        std::cerr << "SettingsMenu: Failed to load font: " << fontPath << '\n';
        return false;
    }

    // Store reference to input controller (for reading/writing bindings)
    m_inputController = inputController;

    // Create title
    m_titleText = std::make_unique<sf::Text>(m_font);
    m_titleText->setString("KEY BINDINGS");
    m_titleText->setCharacterSize(40);
    m_titleText->setFillColor(sf::Color::White);
    m_titleText->setPosition(sf::Vector2f(750.f, 50.f));

    // Build the key binding UI
    createKeyBindingRows();

    return true;
}

void SettingsMenu::createKeyBindingRows()
{
    /*
     * Layout:
     * Each row has:
     * - Button (left): Shows action name, click to rebind
     * - Label (right): Shows current key binding
     */

    float currentY = START_Y;
    float xPos = 700.f;

    // Define which actions can be rebound
    std::vector<std::pair<std::string, InputAction>> rebindableActions = {
        {"Move Up", InputAction::MoveUp},
        {"Move Down", InputAction::MoveDown},
        {"Move Left", InputAction::MoveLeft},
        {"Move Right", InputAction::MoveRight},
        {"Pause", InputAction::Pause}
    };

    // Create a row for each action
    for (const auto& [label, action] : rebindableActions)
    {
        KeyBindingRow row;
        row.action = action;

        // Create button (left side)
        row.button = std::make_unique<ButtonComponent>();
        row.button->initialize(m_font, label);
        row.button->setPosition(sf::Vector2f(xPos, currentY));
        row.button->setSize(sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT));

        // Capture 'this' and 'action' in lambda
        row.button->setCallback([this, action]() {
            m_waitingForKey = true;
            m_actionToRebind = action;
            std::cout << "Waiting for key press..." << std::endl;
            });

        // Create key label (right side)
        row.keyLabel = std::make_unique<sf::Text>(m_font);
        row.keyLabel->setCharacterSize(20);
        row.keyLabel->setFillColor(sf::Color::Yellow);
        row.keyLabel->setPosition(sf::Vector2f(xPos + BUTTON_WIDTH + 20.f, currentY + 12.f));

        // Add row to list
        m_keyBindingRows.push_back(std::move(row));

        // Move to next row position
        currentY += BUTTON_HEIGHT + BUTTON_SPACING;
    }

    // Create back button below all rows
    m_backButton = std::make_unique<ButtonComponent>();
    m_backButton->initialize(m_font, "Back");
    m_backButton->setPosition(sf::Vector2f(xPos, currentY + 30.f));
    m_backButton->setSize(sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT));
    m_backButton->setNormalColor(sf::Color(100, 50, 50));
    m_backButton->setHoverColor(sf::Color(150, 70, 70));
    m_backButton->setCallback([this]() {
        if (m_backCallback)
            m_backCallback();
        });

    // Initialize all key labels with current bindings
    updateKeyLabels();
}

void SettingsMenu::update(const sf::Vector2f& mousePos, bool mousePressed)
{
    // Don't allow button clicks while waiting for key input
    if (!m_visible || m_waitingForKey)
        return;

    // Update all rebind buttons
    for (auto& row : m_keyBindingRows)
    {
        row.button->update(mousePos, mousePressed);
    }

    // Update back button
    m_backButton->update(mousePos, mousePressed);
}

void SettingsMenu::handleKeyPress(sf::Keyboard::Key key)
{
    /*
     * Called from Game.cpp when a key is pressed
     * Only processes input when in "waiting for key" mode
     */

    if (!m_waitingForKey || !m_inputController)
        return;

    // ESC cancels rebinding
    if (key == sf::Keyboard::Key::Escape)
    {
        m_waitingForKey = false;
        std::cout << "Rebinding cancelled" << std::endl;
        return;
    }

    // Bind the key to the action
    m_inputController->bindKey(m_actionToRebind, key);
    m_waitingForKey = false;

    // Update the displayed key names
    updateKeyLabels();

    std::cout << "Key rebound successfully!" << std::endl;
}

void SettingsMenu::render(sf::RenderTarget& target) const
{
    if (!m_visible)
        return;

    // Draw semi-transparent dark background overlay
    sf::RectangleShape background(sf::Vector2f(1920.f, 1080.f));
    background.setFillColor(sf::Color(0, 0, 0, 200));  // 200/255 = ~78% opacity
    target.draw(background);

    // Draw title
    if (m_titleText)
        target.draw(*m_titleText);

    // Draw all key binding rows (button + key label)
    for (const auto& row : m_keyBindingRows)
    {
        row.button->render(target);
        target.draw(*row.keyLabel);
    }

    // Draw back button
    m_backButton->render(target);

    // Draw instruction text when waiting for key input
    if (m_waitingForKey)
    {
        sf::Text waitText(m_font);
        waitText.setString("Press a key... (ESC to cancel)");
        waitText.setCharacterSize(30);
        waitText.setFillColor(sf::Color::Yellow);
        waitText.setPosition(sf::Vector2f(650.f, 800.f));
        target.draw(waitText);
    }
}

void SettingsMenu::updateKeyLabels()
{
    /*
     * Read current key bindings from InputController
     * and update the displayed text
     */

    if (!m_inputController)
        return;

    for (auto& row : m_keyBindingRows)
    {
        // Get the currently bound key for this action
        sf::Keyboard::Key key = m_inputController->getKeyBinding(row.action);

        // Convert key enum to readable string
        std::string keyName = getKeyName(key);

        // Update the label
        row.keyLabel->setString(keyName);
    }
}

std::string SettingsMenu::getKeyName(sf::Keyboard::Key key) const
{
    /*
     * Convert SFML key enum to human-readable string
     * Maps most common keys - extend as needed
     */

    switch (key)
    {
        // Letters
    case sf::Keyboard::Key::A: return "A";
    case sf::Keyboard::Key::B: return "B";
    case sf::Keyboard::Key::C: return "C";
    case sf::Keyboard::Key::D: return "D";
    case sf::Keyboard::Key::E: return "E";
    case sf::Keyboard::Key::F: return "F";
    case sf::Keyboard::Key::G: return "G";
    case sf::Keyboard::Key::H: return "H";
    case sf::Keyboard::Key::I: return "I";
    case sf::Keyboard::Key::J: return "J";
    case sf::Keyboard::Key::K: return "K";
    case sf::Keyboard::Key::L: return "L";
    case sf::Keyboard::Key::M: return "M";
    case sf::Keyboard::Key::N: return "N";
    case sf::Keyboard::Key::O: return "O";
    case sf::Keyboard::Key::P: return "P";
    case sf::Keyboard::Key::Q: return "Q";
    case sf::Keyboard::Key::R: return "R";
    case sf::Keyboard::Key::S: return "S";
    case sf::Keyboard::Key::T: return "T";
    case sf::Keyboard::Key::U: return "U";
    case sf::Keyboard::Key::V: return "V";
    case sf::Keyboard::Key::W: return "W";
    case sf::Keyboard::Key::X: return "X";
    case sf::Keyboard::Key::Y: return "Y";
    case sf::Keyboard::Key::Z: return "Z";

        // Special keys
    case sf::Keyboard::Key::Space: return "Space";
    case sf::Keyboard::Key::Enter: return "Enter";
    case sf::Keyboard::Key::Escape: return "Escape";
    case sf::Keyboard::Key::LShift: return "LShift";
    case sf::Keyboard::Key::RShift: return "RShift";
    case sf::Keyboard::Key::LControl: return "LCtrl";
    case sf::Keyboard::Key::RControl: return "RCtrl";

        // Arrow keys
    case sf::Keyboard::Key::Up: return "Up";
    case sf::Keyboard::Key::Down: return "Down";
    case sf::Keyboard::Key::Left: return "Left";
    case sf::Keyboard::Key::Right: return "Right";

    default: return "Unknown";
    }
}