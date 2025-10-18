#include "SettingsMenu.h"
#include <iostream>

SettingsMenu::SettingsMenu()
    : m_inputController(nullptr)
    , m_screenSettings(nullptr)
    , m_visible(false)
    , m_waitingForKey(false)
    , m_actionToRebind(InputAction::MoveUp)
    , m_backCallback(nullptr)
    , m_applyCallback(nullptr)
    , m_startX(0.f)
    , m_startY(0.f)
    , m_keyBindingsY(0.f)
    , m_videoSettingsY(0.f)
{
}

bool SettingsMenu::initialize(const std::string& fontPath, InputController* inputController, ScreenSettings* screenSettings)
{
    if (!m_font.openFromFile(fontPath))
    {
        std::cerr << "SettingsMenu: Failed to load font: " << fontPath << '\n';
        return false;
    }

    m_inputController = inputController;
    m_screenSettings = screenSettings;

    // Create title
    m_titleText = std::make_unique<sf::Text>(m_font);
    m_titleText->setString("SETTINGS");
    m_titleText->setCharacterSize(40);
    m_titleText->setFillColor(sf::Color::White);

    // Create section headers
    m_keyBindingsHeader = std::make_unique<sf::Text>(m_font);
    m_keyBindingsHeader->setString("KEY BINDINGS");
    m_keyBindingsHeader->setCharacterSize(30);
    m_keyBindingsHeader->setFillColor(sf::Color(200, 200, 200));

    m_videoHeader = std::make_unique<sf::Text>(m_font);
    m_videoHeader->setString("VIDEO SETTINGS");
    m_videoHeader->setCharacterSize(30);
    m_videoHeader->setFillColor(sf::Color(200, 200, 200));

    // Build UI sections
    createKeyBindingSection();
    createVideoSection();

    // Initial layout for 1920x1080
    updateLayout(1920, 1080);

    return true;
}

void SettingsMenu::createKeyBindingSection()
{
    std::vector<std::pair<std::string, InputAction>> rebindableActions = {
        {"Move Up", InputAction::MoveUp},
        {"Move Down", InputAction::MoveDown},
        {"Move Left", InputAction::MoveLeft},
        {"Move Right", InputAction::MoveRight},
        {"Sprint", InputAction::Sprint},
        {"Inventory", InputAction::Inventory},
        {"Pause", InputAction::Pause}
    };

    for (const auto& [label, action] : rebindableActions)
    {
        KeyBindingRow row;
        row.action = action;

        row.button = std::make_unique<ButtonComponent>();
        row.button->initialize(m_font, label);
        row.button->setSize(sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT));
        row.button->setCallback([this, action]() {
            m_waitingForKey = true;
            m_actionToRebind = action;
            std::cout << "Waiting for key press..." << std::endl;
            });

        row.keyLabel = std::make_unique<sf::Text>(m_font);
        row.keyLabel->setCharacterSize(20);
        row.keyLabel->setFillColor(sf::Color::Yellow);

        m_keyBindingRows.push_back(std::move(row));
    }

    updateKeyLabels();
}

void SettingsMenu::createVideoSection()
{
    // Resolution label
    m_resolutionLabel = std::make_unique<sf::Text>(m_font);
    m_resolutionLabel->setString("Resolution:");
    m_resolutionLabel->setCharacterSize(24);
    m_resolutionLabel->setFillColor(sf::Color::White);

    m_resolutionValue = std::make_unique<sf::Text>(m_font);
    m_resolutionValue->setCharacterSize(24);
    m_resolutionValue->setFillColor(sf::Color::Yellow);

    // Resolution navigation buttons
    m_resolutionPrevBtn = std::make_unique<ButtonComponent>();
    m_resolutionPrevBtn->initialize(m_font, "<");
    m_resolutionPrevBtn->setSize(sf::Vector2f(50.f, 40.f));
    m_resolutionPrevBtn->setCallback([this]() { onResolutionPrev(); });

    m_resolutionNextBtn = std::make_unique<ButtonComponent>();
    m_resolutionNextBtn->initialize(m_font, ">");
    m_resolutionNextBtn->setSize(sf::Vector2f(50.f, 40.f));
    m_resolutionNextBtn->setCallback([this]() { onResolutionNext(); });

    // Fullscreen toggle
    m_fullscreenLabel = std::make_unique<sf::Text>(m_font);
    m_fullscreenLabel->setString("Fullscreen:");
    m_fullscreenLabel->setCharacterSize(24);
    m_fullscreenLabel->setFillColor(sf::Color::White);

    m_fullscreenToggleBtn = std::make_unique<ButtonComponent>();
    m_fullscreenToggleBtn->initialize(m_font, "OFF");
    m_fullscreenToggleBtn->setSize(sf::Vector2f(100.f, 40.f));
    m_fullscreenToggleBtn->setCallback([this]() { onFullscreenToggle(); });

    // Apply and Back buttons
    m_applyButton = std::make_unique<ButtonComponent>();
    m_applyButton->initialize(m_font, "Apply");
    m_applyButton->setSize(sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT));
    m_applyButton->setNormalColor(sf::Color(50, 100, 50));
    m_applyButton->setHoverColor(sf::Color(70, 150, 70));
    m_applyButton->setCallback([this]() { onApply(); });

    m_backButton = std::make_unique<ButtonComponent>();
    m_backButton->initialize(m_font, "Back");
    m_backButton->setSize(sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT));
    m_backButton->setNormalColor(sf::Color(100, 50, 50));
    m_backButton->setHoverColor(sf::Color(150, 70, 70));
    m_backButton->setCallback([this]() { onBack(); });

    updateVideoLabels();
}

void SettingsMenu::updateLayout(unsigned int screenWidth, unsigned int screenHeight)
{
    // Calculate positions relative to screen size
    float centerX = screenWidth * 0.4f;
    m_startX = centerX - BUTTON_WIDTH / 2.0f;
    m_startY = screenHeight * 0.08f;

    // Title
    m_titleText->setPosition(sf::Vector2f(centerX - 100.f, m_startY));

    // Key bindings section
    m_keyBindingsY = m_startY + 80.f;
    m_keyBindingsHeader->setPosition(sf::Vector2f(m_startX, m_keyBindingsY));

    float currentY = m_keyBindingsY + 50.f;
    for (auto& row : m_keyBindingRows)
    {
        row.button->setPosition(sf::Vector2f(m_startX, currentY));
        row.keyLabel->setPosition(sf::Vector2f(m_startX + BUTTON_WIDTH + 20.f, currentY + 12.f));
        currentY += BUTTON_HEIGHT + BUTTON_SPACING;
    }

    // Video settings section
    m_videoSettingsY = currentY + SECTION_SPACING;
    m_videoHeader->setPosition(sf::Vector2f(m_startX, m_videoSettingsY));

    currentY = m_videoSettingsY + 50.f;

    // Resolution row
    m_resolutionLabel->setPosition(sf::Vector2f(m_startX, currentY + 10.f));
    m_resolutionPrevBtn->setPosition(sf::Vector2f(m_startX + 150.f, currentY));
    m_resolutionValue->setPosition(sf::Vector2f(m_startX + 210.f, currentY + 10.f));
    m_resolutionNextBtn->setPosition(sf::Vector2f(m_startX + 350.f, currentY));
    currentY += 60.f;

    // Fullscreen row
    m_fullscreenLabel->setPosition(sf::Vector2f(m_startX, currentY + 10.f));
    m_fullscreenToggleBtn->setPosition(sf::Vector2f(m_startX + 150.f, currentY));
    currentY += 80.f;

    // Bottom buttons
    m_applyButton->setPosition(sf::Vector2f(m_startX, currentY));
    m_backButton->setPosition(sf::Vector2f(m_startX, currentY + BUTTON_HEIGHT + 15.f));
}

void SettingsMenu::update(const sf::Vector2f& mousePos, bool mousePressed)
{
    if (!m_visible || m_waitingForKey)
        return;

    // Update key binding buttons
    for (auto& row : m_keyBindingRows)
    {
        row.button->update(mousePos, mousePressed);
    }

    // Update video setting buttons
    m_resolutionPrevBtn->update(mousePos, mousePressed);
    m_resolutionNextBtn->update(mousePos, mousePressed);
    m_fullscreenToggleBtn->update(mousePos, mousePressed);
    m_applyButton->update(mousePos, mousePressed);
    m_backButton->update(mousePos, mousePressed);
}

void SettingsMenu::handleKeyPress(sf::Keyboard::Key key)
{
    if (!m_waitingForKey || !m_inputController)
        return;

    if (key == sf::Keyboard::Key::Escape)
    {
        m_waitingForKey = false;
        std::cout << "Rebinding cancelled" << std::endl;
        return;
    }

    m_inputController->bindKey(m_actionToRebind, key);
    m_waitingForKey = false;
    updateKeyLabels();
    std::cout << "Key rebound successfully!" << std::endl;
}

void SettingsMenu::render(sf::RenderTarget& target) const
{
    if (!m_visible)
        return;

    // Dark overlay
    sf::RectangleShape background(sf::Vector2f(target.getSize()));
    background.setFillColor(sf::Color(0, 0, 0, 200));
    target.draw(background);

    // Title
    target.draw(*m_titleText);

    // Key bindings section
    target.draw(*m_keyBindingsHeader);
    for (const auto& row : m_keyBindingRows)
    {
        row.button->render(target);
        target.draw(*row.keyLabel);
    }

    // Video settings section
    target.draw(*m_videoHeader);
    target.draw(*m_resolutionLabel);
    target.draw(*m_resolutionValue);
    m_resolutionPrevBtn->render(target);
    m_resolutionNextBtn->render(target);

    target.draw(*m_fullscreenLabel);
    m_fullscreenToggleBtn->render(target);

    // Bottom buttons
    m_applyButton->render(target);
    m_backButton->render(target);

    // Waiting for key prompt
    if (m_waitingForKey)
    {
        sf::Text waitText(m_font);
        waitText.setString("Press a key... (ESC to cancel)");
        waitText.setCharacterSize(30);
        waitText.setFillColor(sf::Color::Yellow);
        waitText.setPosition(sf::Vector2f(m_startX - 50.f, m_videoSettingsY + 300.f));
        target.draw(waitText);
    }
}

void SettingsMenu::setVisible(bool visible)
{
    m_visible = visible;
    if (visible)
    {
        updateVideoLabels();
    }
}

void SettingsMenu::updateKeyLabels()
{
    if (!m_inputController)
        return;

    for (auto& row : m_keyBindingRows)
    {
        sf::Keyboard::Key key = m_inputController->getKeyBinding(row.action);
        row.keyLabel->setString(getKeyName(key));
    }
}

void SettingsMenu::updateVideoLabels()
{
    if (!m_screenSettings)
        return;

    // Update resolution display
    const auto& res = m_screenSettings->getCurrentResolution();
    m_resolutionValue->setString(res.displayName);

    // Update fullscreen button
    m_fullscreenToggleBtn->setText(m_screenSettings->isFullscreen() ? "ON" : "OFF");
}

void SettingsMenu::onResolutionPrev()
{
    if (!m_screenSettings)
        return;

    int currentIndex = m_screenSettings->getCurrentResolutionIndex();
    int newIndex = currentIndex - 1;

    if (newIndex < 0)
        newIndex = static_cast<int>(m_screenSettings->getAvailableResolutions().size()) - 1;

    m_screenSettings->setResolution(newIndex);
    updateVideoLabels();
}

void SettingsMenu::onResolutionNext()
{
    if (!m_screenSettings)
        return;

    int currentIndex = m_screenSettings->getCurrentResolutionIndex();
    int newIndex = currentIndex + 1;

    if (newIndex >= static_cast<int>(m_screenSettings->getAvailableResolutions().size()))
        newIndex = 0;

    m_screenSettings->setResolution(newIndex);
    updateVideoLabels();
}

void SettingsMenu::onFullscreenToggle()
{
    if (!m_screenSettings)
        return;

    m_screenSettings->toggleFullscreen();
    updateVideoLabels();
}

void SettingsMenu::onApply()
{
    if (m_applyCallback)
        m_applyCallback();
}

void SettingsMenu::onBack()
{
    if (m_backCallback)
        m_backCallback();
}

std::string SettingsMenu::getKeyName(sf::Keyboard::Key key) const
{
    switch (key)
    {
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
    case sf::Keyboard::Key::Space: return "Space";
    case sf::Keyboard::Key::Enter: return "Enter";
    case sf::Keyboard::Key::Escape: return "Escape";
    case sf::Keyboard::Key::LShift: return "LShift";
    case sf::Keyboard::Key::RShift: return "RShift";
    case sf::Keyboard::Key::LControl: return "LCtrl";
    case sf::Keyboard::Key::RControl: return "RCtrl";
    case sf::Keyboard::Key::Up: return "Up";
    case sf::Keyboard::Key::Down: return "Down";
    case sf::Keyboard::Key::Left: return "Left";
    case sf::Keyboard::Key::Right: return "Right";
    default: return "Unknown";
    }
}