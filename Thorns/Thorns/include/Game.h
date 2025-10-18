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

    // ========== Utility ==========
    sf::Vector2f getMousePosition() const;

    // ========== SFML Window ==========
    sf::RenderWindow m_window;

    // ========== Game State ==========
    bool m_exitGame;      // Set to true to close window
    bool m_gameValid;     // False if initialization failed
    bool m_mousePressed;  // Track left mouse button state

    // ========== Systems ==========
    GameStateManager m_stateManager;  // Manages state transitions
    InputController m_input;          // Handles keyboard/gamepad input
    ScreenSettings m_screenSettings;  // Manages resolution and fullscreen

    // ========== UI ==========
    Menu m_mainMenu;
    Menu m_pauseMenu;
    SettingsMenu m_settingsMenu;

    // ========== Game Objects ==========
    Player m_player;
    Enemy m_enemy;
};

#endif