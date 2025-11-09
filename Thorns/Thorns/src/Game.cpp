#include "Game.h"
#include "AssetPaths.h"
#include <iostream>

Game::Game()
    : m_window(sf::VideoMode{ sf::Vector2u{1920U, 1080U}, 32U }, "THORNS")
    , m_gameView(sf::FloatRect(sf::Vector2f(0.f, 0.f), sf::Vector2f(1920.f, 1080.f)))
    , m_uiView(sf::FloatRect(sf::Vector2f(0.f, 0.f), sf::Vector2f(1920.f, 1080.f)))
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
        std::cerr << "Failed to load player!" << std::endl;
        return false;
    }

    // Initialize enemy entity
    if (!m_enemy.initialize(Assets::Textures::ENEMY_SHIP))
    {
        std::cerr << "Failed to load enemy!" << std::endl;
        return false;
    }

    // Initialize UI (menus load fonts but don't create buttons yet)
    if (!m_mainMenu.initialize(Assets::Fonts::JERSEY_20))
    {
        std::cerr << "Failed to initialize main menu!" << std::endl;
        return false;
    }

    if (!m_pauseMenu.initialize(Assets::Fonts::JERSEY_20))
    {
        std::cerr << "Failed to initialize pause menu!" << std::endl;
        return false;
    }

    // Settings menu needs access to input controller and screen settings
    if (!m_settingsMenu.initialize(Assets::Fonts::JERSEY_20, &m_input, &m_screenSettings))
    {
        std::cerr << "Failed to initialize settings menu!" << std::endl;
        return false;
    }

    // Now that resources are loaded, configure the menus
    setupMenus();

    // Generate the starting map
    generateMap();

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

// Step 1 for map generation
// Use settings, initalise the map
void Game::generateMap()
{
    // Configure generation settings (Defaults in base class, change here for testing)
    m_mapSettings.mapWidth = 40;
    m_mapSettings.mapHeight = 40;
    m_mapSettings.tileSize = 64.f;
    m_mapSettings.voronoiSites = 10;
    m_mapSettings.minSiteDistance = 400.0f;  // Minimum 400 pixels between sites
    m_currentSeed = 12345;
    m_mapSettings.seed = m_currentSeed;

    // POI spawning configuration
    m_mapSettings.numVillages = 1;
    m_mapSettings.numFarms = 2;

    // Generating map
    m_map = m_mapGenerator.generate(m_mapSettings);

    // Position player at map center (where hideout is)
    sf::Vector2f mapCenter = m_map->getWorldSize();
    mapCenter.x /= 2.f;
    mapCenter.y /= 2.f;
    m_player.setPosition(mapCenter);

    std::cout << "Map generated! World size: "
        << m_map->getWorldSize().x << "x" << m_map->getWorldSize().y << " pixels\n";
    std::cout << "TO BE REMOVED: Press 'R' during gameplay to regenerate map with new seed\n";

}

void Game::regenerateMap()
{
    std::cout << "\n========== REGENERATING MAP ==========\n";

    // Increment seed for new generation
    ++m_currentSeed;
    m_mapSettings.seed = m_currentSeed;

    // Regenerate existing map (reuses memory instead of allocating new)
    m_mapGenerator.regenerate(m_map.get(), m_mapSettings);

    // Reset player position to center (for when night time is added and over need "flash" screen with day also)
    sf::Vector2f mapCenter = m_map->getWorldSize();
    mapCenter.x /= 2.f;
    mapCenter.y /= 2.f;
    m_player.setPosition(mapCenter);

    std::cout << "Map regenerated with seed " << m_currentSeed << "!\n";
    std::cout << "======================================\n\n";
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
        updateMainMenu();
        break;
    case GameState::Settings:
        updateSettings();
        break;
    case GameState::Playing:
        updatePlaying(deltaTime);
        break;
    case GameState::Paused:
        updatePaused();
        break;
    case GameState::GameOver:
        break;
    }
}

void Game::updateMainMenu()
{
    // Main menu only needs mouse input for button interaction
    m_mainMenu.update(m_input);
}

void Game::updateSettings()
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
    m_player.updateWithInput(deltaTime, m_input, getMouseWorldPosition());

    //==============================================================================================<<<<< REMOVE THE BELOW WHEN DONT HAVE TO DEBUG
    // Check for map regeneration (R key)
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::R))
    {
        regenerateMap();
    }

    // Update other entities
    m_enemy.update(deltaTime);

    updateCamera();
}

void Game::updatePaused()
{
    // Pause menu only needs mouse input for button interaction
    m_pauseMenu.update(m_input);
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// ======================== PN REMOVE THE SITES USED FOR RENDERING WHEN NEAR DONE IN TESTING ========================================
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
            m_window.setView(m_gameView);
            if (m_map)
                m_map->render(m_window);
            m_mapGenerator.getVoronoiDiagram()->renderDebug(m_window);
            m_player.render(m_window);
            m_enemy.render(m_window);
        }
        m_window.setView(m_uiView);
        m_settingsMenu.render(m_window);  // Draw settings over game
        break;

    case GameState::Playing:
        // Normal gameplay rendering
        m_window.setView(m_gameView);

        if (m_map)
            m_map->render(m_window);
        m_mapGenerator.getVoronoiDiagram()->renderDebug(m_window);
        m_player.render(m_window);
        m_enemy.render(m_window);
        break;

    case GameState::Paused:
        // Show frozen game in background with pause menu overlay
        m_window.setView(m_gameView);
        if (m_map)
            m_map->render(m_window);
        m_mapGenerator.getVoronoiDiagram()->renderDebug(m_window);
        m_player.render(m_window);
        m_enemy.render(m_window);

        m_window.setView(m_uiView);
        m_pauseMenu.render(m_window);
        break;

    case GameState::GameOver:

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

void Game::updateCamera()
{
    // Center camera on player
    sf::Vector2f playerPos = m_player.getPosition();

    // Clamp camera to map bounds so we don't see outside the map
    sf::Vector2f clampedPos = clampCameraToMapBounds(playerPos);

    m_gameView.setCenter(clampedPos);
}

sf::Vector2f Game::clampCameraToMapBounds(const sf::Vector2f& targetPos)
{
    if (!m_map)
        return targetPos;

    sf::Vector2f viewSize = m_gameView.getSize();
    sf::Vector2f mapSize = m_map->getWorldSize();

    // Calculate half view size
    float halfWidth = viewSize.x / 2.f;
    float halfHeight = viewSize.y / 2.f;

    // Clamp camera center so view edges don't go outside map
    float clampedX = std::max(halfWidth, std::min(targetPos.x, mapSize.x - halfWidth));
    float clampedY = std::max(halfHeight, std::min(targetPos.y, mapSize.y - halfHeight));

    return sf::Vector2f(clampedX, clampedY);
}
// ========== Utility ==========

sf::Vector2f Game::getMousePosition() const
{
    // Convert window-relative mouse position to Vector2f for button checks
    return sf::Vector2f(sf::Mouse::getPosition(m_window));
}

sf::Vector2f Game::getMouseWorldPosition() const
{
    // Convert screen coordinates to world coordinates using game view
    sf::Vector2i mousePixel = sf::Mouse::getPosition(m_window);
    return m_window.mapPixelToCoords(mousePixel, m_gameView);
}
