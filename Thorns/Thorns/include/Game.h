#ifndef GAME_HPP
#define GAME_HPP

#include <SFML/Graphics.hpp>
#include "Player.h"
#include "Enemy.h"
#include "InputController.h"
#include "Menu.h"
#include "SettingsMenu.h"
#include "GameStateManager.h"
#include "ScreenSettings.h"
#include "IUpdatable.h"
#include "Map.h"
#include "MapGenerator.h"
#include "CollisionManager.h"

class Game : IUpdatable
{
public:
    Game();
    ~Game();

    // Main game loop - runs until window closes
    void run();

private:
    // ========== Core Loop ==========
    void processEvents();              // Handle window events (close, mouse, keys)
    void update(sf::Time deltaTime) override;   // Update game logic at fixed timestep
    void render();                     // Render current game state

    // ========== Initialization ==========
    bool initializeGame();    // Load all resources and setup game objects
    void setupMenus();        // Configure menu items and callbacks


    // ========== State Callbacks ==========
    // Called automatically by GameStateManager when states change
    void onStateEnter(GameState state);
    void onStateExit(GameState state);

    // ========== State Update Methods ==========
    // Each state has its own update logic
    void updateMainMenu();
    void updateSettings();
    void updatePlaying(sf::Time deltaTime);
    void updatePaused();

    // ========== Menu Action Callbacks ==========
    // These are bound to menu buttons via lambdas
    void onStartGame();
    void onOpenSettings();
    void onResumeGame();
    void onQuitGame();
    void onBackToMenu();
    void onBackFromSettings();
    void onApplySettings();

    // ========== Map ==========
    void generateMap();         // Called during game init also
    void regenerateMap();     // Regenerate map with new seed during runtime

    // ========== Camera ========== (might make class later PN)
    void updateCamera();
    sf::Vector2f clampCameraToMapBounds(const sf::Vector2f& targetPos);

    // ========== Utility ==========
    sf::Vector2f getMousePosition() const;
    sf::Vector2f getMouseWorldPosition() const;

    // ========== SFML Window ==========
    sf::RenderWindow m_window;
    sf::View m_gameView;        // Game play camera
    sf::View m_uiView;          // Ui set to different view 

    // ========== Game State ==========
    bool m_exitGame;      // Set to true to close window
    bool m_gameValid;     // False if initialization failed
    bool m_mousePressed;  // Track left mouse button state

    // ========== Systems ==========
    GameStateManager m_stateManager;  // Manages state transitions
    CollisionManager m_collisionManager;    // Manages Collision between objects and entities. 
    InputController m_input;          // Handles keyboard/gamepad input
    ScreenSettings m_screenSettings;  // Manages resolution and fullscreen

    // ========== UI ==========
    Menu m_mainMenu;
    Menu m_pauseMenu;
    SettingsMenu m_settingsMenu;

    // ========== Game Objects ==========
    Player m_player;
    Enemy m_enemy;

    // ========== World ==========
    std::unique_ptr<Map> m_map;
    MapGenerator m_mapGenerator;
    MapGenerator::GenerationSettings m_mapSettings; // Store settings for when I add saving
    unsigned int m_currentSeed; // Track current seed
};

#endif