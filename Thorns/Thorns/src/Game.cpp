#include "Game.h"
#include "AssetPaths.h"
#include <iostream>

Game::Game()
    : m_window(sf::VideoMode{ sf::Vector2u{1920U, 1080U}, 32U }, "THORNS")
    , m_exitGame(false)
    , m_gameValid(false)
    , m_mousePressed(false)
    , m_screenSettings()  // Initialize screen settings
{
    // Register state change callbacks with the state manager
    m_stateManager.setOnStateEnter([this](GameState state) { onStateEnter(state); });
    m_stateManager.setOnStateExit([this](GameState state) { onStateExit(state); });

    // Load all resources and setup game
    m_gameValid = initializeGame();
}

Game::~Game()
{
}

bool Game::initializeGame()
{
    // Initialize player entity
    if (!m_player.initialize(Assets::Textures::PLAYER_ATLAS))
    {
        std::cout << "Failed to load player!" << std::endl;
        return false;
    }

    // Initialize enemy entity
    if (!m_enemy.initialize(Assets::Textures::ENEMY_SHIP))
    {
        std::cout << "Failed to load enemy!" << std::endl;
        return false;
    }

    // Initialize UI (menus load fonts but don't create buttons yet)
    if (!m_mainMenu.initialize(Assets::Fonts::JERSEY_20))
    {
        std::cout << "Failed to initialize main menu!" << std::endl;
        return false;
    }

    if (!m_pauseMenu.initialize(Assets::Fonts::JERSEY_20))
    {
        std::cout << "Failed to initialize pause menu!" << std::endl;
        return false;
    }

    // Settings menu needs access to input controller and screen settings
    if (!m_settingsMenu.initialize(Assets::Fonts::JERSEY_20, &m_input, &m_screenSettings))
    {
        std::cout << "Failed to initialize settings menu!" << std::endl;
        return false;
    }

    // Now that resources are loaded, configure the menus
    setupMenus();

    std::cout << "Game initialized successfully!" << std::endl;
    return true;
}

void Game::setupMenus()
{
    // ===== Main Menu =====
    m_mainMenu.setTitle("THORNS");
    m_mainMenu.setPosition(sf::Vector2f(800.f, 300.f));
    m_mainMenu.addButton("Start Game", [this]() { onStartGame(); });
    m_mainMenu.addButton("Settings", [this]() { onOpenSettings(); });
    m_mainMenu.addButton("Quit", [this]() { onQuitGame(); });

    // ===== Pause Menu =====
    m_pauseMenu.setTitle("PAUSED");
    m_pauseMenu.setPosition(sf::Vector2f(800.f, 300.f));
    m_pauseMenu.addButton("Resume", [this]() { onResumeGame(); });
    m_pauseMenu.addButton("Settings", [this]() { onOpenSettings(); });
    m_pauseMenu.addButton("Main Menu", [this]() { onBackToMenu(); });
    m_pauseMenu.addButton("Quit", [this]() { onQuitGame(); });
    m_pauseMenu.setVisible(false);  // Hide until paused

    // ===== Settings Menu =====
    m_settingsMenu.setApplyCallback([this]() { onApplySettings(); });
    m_settingsMenu.setBackCallback([this]() {onBackFromSettings(); });
    m_settingsMenu.setVisible(false);
}

void Game::run()
{
    // Early exit if initialization failed
    if (!m_gameValid)
    {
        std::cout << "Game failed to initialize. Exiting..." << std::endl;
        std::cout << "Press Enter to close..." << std::endl;
        std::cin.get();
        return;
    }

    // ===== Fixed Timestep Game Loop =====
    // Updates happen at fixed 60 FPS, rendering happens as fast as possible
    sf::Clock clock;
    sf::Time timeSinceLastUpdate = sf::Time::Zero;
    const float fps = 60.0f;
    sf::Time timePerFrame = sf::seconds(1.0f / fps);

    while (m_window.isOpen())
    {
        // Handle events (window close, input, etc.)
        processEvents();

        // Accumulate time since last update
        timeSinceLastUpdate += clock.restart();

        // Update at fixed timestep (may update multiple times per frame if lagging)
        while (timeSinceLastUpdate > timePerFrame)
        {
            timeSinceLastUpdate -= timePerFrame;
            m_input.update(m_window);              // Update input state
            update(timePerFrame);          // Update game logic
        }

        // Render as fast as possible
        render();
    }
}

void Game::processEvents()
{
    // Poll all pending events from SFML
    while (const std::optional newEvent = m_window.pollEvent())
    {
        // Check for window close
        if (newEvent->is<sf::Event::Closed>())
        {
            m_exitGame = true;
        }
        // Track left mouse button press
        else if (const auto* mouseButtonPressed = newEvent->getIf<sf::Event::MouseButtonPressed>())
        {
            if (mouseButtonPressed->button == sf::Mouse::Button::Left)
            {
                m_mousePressed = true;
            }
        }
        // Track left mouse button release
        else if (const auto* mouseButtonReleased = newEvent->getIf<sf::Event::MouseButtonReleased>())
        {
            if (mouseButtonReleased->button == sf::Mouse::Button::Left)
            {
                m_mousePressed = false;
            }
        }
        // Handle key presses for rebinding
        else if (const auto* keyPressed = newEvent->getIf<sf::Event::KeyPressed>())
        {
            // Only handle key events when settings menu is waiting for input
            if (m_stateManager.getCurrentState() == GameState::Settings &&
                m_settingsMenu.isWaitingForKey())
            {
                m_settingsMenu.handleKeyPress(keyPressed->code);
            }
        }
    }
}

void Game::update(sf::Time deltaTime)
{
    // Handle exit request
    if (m_exitGame)
    {
        m_window.close();
        return;
    }

    // Delegate to state-specific update method
    switch (m_stateManager.getCurrentState())
    {
    case GameState::MainMenu:
        updateMainMenu(deltaTime);
        break;
    case GameState::Settings:
        updateSettings(deltaTime);
        break;
    case GameState::Playing:
        updatePlaying(deltaTime);
        break;
    case GameState::Paused:
        updatePaused(deltaTime);
        break;
    case GameState::GameOver:
        updateGameOver(deltaTime);
        break;
    }
}

void Game::updateMainMenu(sf::Time deltaTime)
{
    // Main menu only needs mouse input for button interaction
    m_mainMenu.update(m_input);
}

void Game::updateSettings(sf::Time deltaTime)
{
    // Settings menu handles button clicks and key rebinding
    m_settingsMenu.update(getMousePosition(), m_mousePressed);

    // Allow Escape to exit settings (but not while rebinding a key)
    if (m_input.wasJustPressed(InputAction::Cancel) && !m_settingsMenu.isWaitingForKey())
    {
        onBackFromSettings();
    }
}

void Game::updatePlaying(sf::Time deltaTime)
{
    // Check for pause input
    if (m_input.wasJustPressed(InputAction::Pause) || m_input.wasJustPressed(InputAction::Menu))
    {
        m_stateManager.pushState(GameState::Paused);  // Push so we can pop back
        return;
    }

    // Update player with input and mouse position
    m_player.updateWithInput(deltaTime, m_input, m_input.getMousePosition());

    // Update other entities
    m_enemy.update(deltaTime);
}

void Game::updatePaused(sf::Time deltaTime)
{
    // Pause menu only needs mouse input for button interaction
    m_pauseMenu.update(m_input);
}

void Game::updateGameOver(sf::Time deltaTime)
{
    // TODO: Implement game over logic
}

void Game::render()
{
    m_window.clear(sf::Color::Black);

    // Get current and previous states for context-aware rendering
    GameState currentState = m_stateManager.getCurrentState();
    GameState previousState = m_stateManager.getPreviousState();

    // Render based on current state
    switch (currentState)
    {
    case GameState::MainMenu:
        // Only show main menu
        m_mainMenu.render(m_window);
        break;

    case GameState::Settings:
        // Show game in background if coming from pause/playing
        if (previousState == GameState::Paused || previousState == GameState::Playing)
        {
            m_player.render(m_window);
            m_enemy.render(m_window);
        }
        m_settingsMenu.render(m_window);  // Draw settings over game
        break;

    case GameState::Playing:
        // Normal gameplay rendering
        m_player.render(m_window);
        m_enemy.render(m_window);
        break;

    case GameState::Paused:
        // Show frozen game in background with pause menu overlay
        m_player.render(m_window);
        m_enemy.render(m_window);
        m_pauseMenu.render(m_window);
        break;

    case GameState::GameOver:
        // TODO: Render game over screen
        break;
    }

    m_window.display();
}

// ========== State Transition Callbacks ==========

void Game::onStateEnter(GameState state)
{
    // Show/hide appropriate UI elements when entering a state
    switch (state)
    {
    case GameState::MainMenu:
        m_mainMenu.setVisible(true);
        m_pauseMenu.setVisible(false);
        m_settingsMenu.setVisible(false);
        m_window.setMouseCursorVisible(true);  // Show OS cursor in menus
        m_player.getCursor().setVisible(false); // Hide custom cursor
        break;

    case GameState::Settings:
        m_settingsMenu.setVisible(true);
        m_mainMenu.setVisible(false);
        m_pauseMenu.setVisible(false);
        m_window.setMouseCursorVisible(true);
        m_player.getCursor().setVisible(false);
        break;

    case GameState::Playing:
        // Hide all UI during gameplay
        m_mainMenu.setVisible(false);
        m_pauseMenu.setVisible(false);
        m_settingsMenu.setVisible(false);
        m_window.setMouseCursorVisible(false); // Hide OS cursor
        m_player.getCursor().setVisible(true);  // Show custom cursor
        break;

    case GameState::Paused:
        m_pauseMenu.setVisible(true);
        m_settingsMenu.setVisible(false);
        m_window.setMouseCursorVisible(true);
        m_player.getCursor().setVisible(false);
        break;

    case GameState::GameOver:
        break;
    }
}

void Game::onStateExit(GameState state)
{
    // Optional: Add cleanup logic when exiting states
}

// ========== Menu Action Callbacks ==========

void Game::onStartGame()
{
    // Direct state change (clears state stack)
    m_stateManager.changeState(GameState::Playing);
}

void Game::onOpenSettings()
{
    // Push state (allows returning to previous state)
    m_stateManager.pushState(GameState::Settings);
}

void Game::onResumeGame()
{
    // Pop back to playing state
    m_stateManager.popState();
}

void Game::onQuitGame()
{
    m_exitGame = true;
}

void Game::onBackToMenu()
{
    // Direct change to main menu (abandons game state)
    m_stateManager.changeState(GameState::MainMenu);
}

void Game::onBackFromSettings()
{
    // Pop back to previous state (pause or main menu)
    m_stateManager.popState();
}

void Game::onApplySettings()
{
    // Apply screen settings (this will recreate the window)
    m_screenSettings.applySettings(m_window);

    // Update UI layouts for new resolution
    const auto& res = m_screenSettings.getCurrentResolution();
    m_settingsMenu.updateLayout(res.width, res.height);

    std::cout << "Settings applied!" << std::endl;
}

// ========== Utility ==========

sf::Vector2f Game::getMousePosition() const
{
    // Convert window-relative mouse position to Vector2f for button checks
    return sf::Vector2f(sf::Mouse::getPosition(m_window));
}