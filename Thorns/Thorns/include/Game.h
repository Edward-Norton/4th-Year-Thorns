#ifndef GAME_HPP
#define GAME_HPP

#include <SFML/Graphics.hpp>
#include "Player.h"
#include "EnemyManager.h"
#include "InputController.h"
#include "Menu.h"
#include "SettingsMenu.h"
#include "GameStateManager.h"
#include "ScreenSettings.h"
#include "IUpdatable.h"
#include "Map.h"
#include "MapGenerator.h"
#include "CollisionManager.h"
#include "WorldItemPool.h"
#include "ItemType.h"
#include "DayTimerComponent.h"

class Game : IUpdatable
{
public:
    Game();
    ~Game();

    
    void run();

private:
    
    void processEvents();              
    void update(sf::Time deltaTime) override;   
    void render();                     

    
    bool initializeGame();    
    void setupMenus();        

    
    
    void onStateEnter(GameState state);
    void onStateExit(GameState state);

    
    
    void updateMainMenu();
    void updateSettings();
    void updatePlaying(sf::Time deltaTime);
    void updatePaused();

    
    
    void onStartGame();
    void onOpenSettings();
    void onResumeGame();
    void onQuitGame();
    void onBackToMenu();
    void onBackFromSettings();
    void onApplySettings();

    
    void generateMap();         
    void regenerateMap();     

    
    void updateCamera();
    sf::Vector2f clampCameraToMapBounds(const sf::Vector2f& targetPos);

    
    sf::Vector2f getMousePosition() const;
    sf::Vector2f getMouseWorldPosition() const;

    
    sf::RenderWindow m_window;
    sf::View m_gameView;        
    sf::View m_uiView;          

    
    bool m_exitGame;      
    bool m_gameValid;     
    bool m_mousePressed;  

    
    GameStateManager m_stateManager;  
    CollisionManager m_collisionManager;    
    InputController m_input;          
    ScreenSettings m_screenSettings;  

    
    Menu m_mainMenu;
    Menu m_pauseMenu;
    SettingsMenu m_settingsMenu;
    DayTimerComponent m_dayTimer;

    
    Player m_player;
    EnemyManager m_enemyManager;

    
    std::unique_ptr<Map> m_map;
    MapGenerator m_mapGenerator;
    MapGenerator::GenerationSettings m_mapSettings; 
    unsigned int m_currentSeed; 
    
    WorldItemPool    m_itemPool;
    ItemTypeRegistry m_itemTypeRegistry;

};

#endif