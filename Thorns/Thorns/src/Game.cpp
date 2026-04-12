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
    , m_screenSettings()  
{
    
    m_stateManager.setOnStateEnter([this](GameState state) { onStateEnter(state); });
    m_stateManager.setOnStateExit([this](GameState state) { onStateExit(state); });

    
    m_gameValid = initializeGame();
}

Game::~Game()
{
}

bool Game::initializeGame()
{
    
    if (!m_player.initialize(Assets::Textures::PLAYER_ATLAS))
    {
        std::cerr << "Failed to load player!" << std::endl;
        return false;
    }
    m_player.getInventory().setRegistry(&m_itemTypeRegistry);
    m_player.getInventory().setOnItemDropped([this](ItemType type, sf::Vector2f pos)
    {
            sf::Vector2f spawnPos = m_player.getPosition() + sf::Vector2f(50.f, 0.f);
            m_itemPool.spawn(type, spawnPos, m_itemTypeRegistry);
    });

    
    if (!m_enemyManager.initialize(Assets::Textures::SAV_ENEMY,
        Assets::Textures::CHOMP_ENEMY))
    {
        std::cerr << "Failed to initialize enemy manager!\n";
        return false;
    }

    
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

    
    if (!m_settingsMenu.initialize(Assets::Fonts::JERSEY_20, &m_input, &m_screenSettings))
    {
        std::cerr << "Failed to initialize settings menu!" << std::endl;
        return false;
    }

    if (!m_itemTypeRegistry.loadDefinitions(Assets::Data::ITEMS_ATLAS_DEFINITIONS))
    {
        std::cerr << "Game: Item definitions failed to load\n";
    }

    if (!m_itemPool.initialize(Assets::Textures::ITEMS_ATLAS))
    {
        std::cerr << "Game: Item pool failed to initialize\n";
    }

    if (!m_dayTimer.initialize(60.f, Assets::Fonts::JERSEY_20))
    {
        std::cerr << "Game: DayTimer failed to initialize\n";
    }
    m_dayTimer.setOnExpired([this]() { regenerateMap(); });

    
    setupMenus();

    
    generateMap();

    std::cout << "Game initialized successfully!" << std::endl;
    return true;
}

void Game::setupMenus()
{
    
    m_mainMenu.setTitle("THORNS");
    m_mainMenu.setPosition(sf::Vector2f(800.f, 300.f));
    m_mainMenu.addButton("Start Game", [this]() { onStartGame(); });
    m_mainMenu.addButton("Settings", [this]() { onOpenSettings(); });
    m_mainMenu.addButton("Quit", [this]() { onQuitGame(); });

    
    m_pauseMenu.setTitle("PAUSED");
    m_pauseMenu.setPosition(sf::Vector2f(800.f, 300.f));
    m_pauseMenu.addButton("Resume", [this]() { onResumeGame(); });
    m_pauseMenu.addButton("Settings", [this]() { onOpenSettings(); });
    m_pauseMenu.addButton("Main Menu", [this]() { onBackToMenu(); });
    m_pauseMenu.addButton("Quit", [this]() { onQuitGame(); });
    m_pauseMenu.setVisible(false);  

    
    m_settingsMenu.setApplyCallback([this]() { onApplySettings(); });
    m_settingsMenu.setBackCallback([this]() {onBackFromSettings(); });
    m_settingsMenu.setVisible(false);
}

void Game::generateMap()
{
    
    m_mapSettings.mapWidth = 160;
    m_mapSettings.mapHeight = 160;
    m_mapSettings.tileSize = 64.f;
    m_currentSeed = 12345;
    m_mapSettings.seed = m_currentSeed;

    
    
    
    m_mapSettings.siteMode = MapGenerator::GenerationSettings::ManualSites{ 20 };
    

    
    m_mapSettings.minSiteDistance = 0.0f;

    
    m_mapSettings.numVillages = 1;
    m_mapSettings.numFarms = 2;

    
    m_mapSettings.enableObjectPlacement = true;  
    m_mapSettings.objectFrequency = 0.08;        
    m_mapSettings.objectOctaves = 2;             
    m_mapSettings.objectThreshold = 0.65;        

    
    m_map = m_mapGenerator.generate(m_mapSettings);

    
    sf::Vector2f mapCenter = m_map->getWorldSize();
    mapCenter.x /= 2.f;
    mapCenter.y /= 2.f;
    m_player.setPosition(mapCenter);

    std::cout << "Map generated! World size: "
        << m_map->getWorldSize().x << "x" << m_map->getWorldSize().y << " pixels\n";
    std::cout << "TO BE REMOVED: Press 'R' during gameplay to regenerate map with new seed\n";

    
    m_enemyManager.despawnAll();
    auto spawnPoints = m_mapGenerator.getEnemySpawnPoints(2, 100.f); 
    for (size_t i = 0; i < spawnPoints.size(); ++i)
    {
        
        if (i % 2 == 0)
            m_enemyManager.spawnSavage(spawnPoints[i]);
        else
            m_enemyManager.spawnChomper(spawnPoints[i]);
    }

    m_itemPool.despawnAll();
    
    auto itemSpawnPoints = m_mapGenerator.getItemSpawnPoints(20.0f);

    
    const ItemType lootTable[] = {
        ItemType::Knife, ItemType::Axe, ItemType::Gun,
        ItemType::FirstAid, ItemType::Bandage
    };
    constexpr int lootTableSize = 5;

    for (size_t i = 0; i < itemSpawnPoints.size(); ++i)
    {
        ItemType lootType = lootTable[i % lootTableSize];
        m_itemPool.spawn(lootType, itemSpawnPoints[i], m_itemTypeRegistry);
    }

    m_itemPool.spawn(ItemType::Food, mapCenter + sf::Vector2f(100, -400), m_itemTypeRegistry);
    m_itemPool.spawn(ItemType::Water, mapCenter + sf::Vector2f(150, -400), m_itemTypeRegistry);
    m_itemPool.spawn(ItemType::Bandage, mapCenter + sf::Vector2f(200, -400), m_itemTypeRegistry);
    m_itemPool.spawn(ItemType::FirstAid, mapCenter + sf::Vector2f(250, -400), m_itemTypeRegistry);
    m_itemPool.spawn(ItemType::Knife, mapCenter + sf::Vector2f(300, -400), m_itemTypeRegistry);
    m_itemPool.spawn(ItemType::Axe, mapCenter + sf::Vector2f(350, -400), m_itemTypeRegistry);
    m_itemPool.spawn(ItemType::Gun , mapCenter + sf::Vector2f(400, -400), m_itemTypeRegistry);
}

void Game::regenerateMap()
{
    std::cout << "\n========== REGENERATING MAP ==========\n";

    
    ++m_currentSeed;
    m_mapSettings.seed = m_currentSeed;

    
    m_mapGenerator.regenerate(m_map.get(), m_mapSettings);

    
    sf::Vector2f mapCenter = m_map->getWorldSize();
    mapCenter.x /= 2.f;
    mapCenter.y /= 2.f;
    m_player.setPosition(mapCenter);

    
    m_enemyManager.despawnAll();
    auto spawnPoints = m_mapGenerator.getEnemySpawnPoints(2, 100.f); 
    for (size_t i = 0; i < spawnPoints.size(); ++i)
    {
        
        if (i % 2 == 0)
            m_enemyManager.spawnSavage(spawnPoints[i]);
        else
            m_enemyManager.spawnChomper(spawnPoints[i]);
    }

    m_itemPool.despawnAll();
    
    auto itemSpawnPoints = m_mapGenerator.getItemSpawnPoints(20.0f);

    
    const ItemType lootTable[] = {
        ItemType::Knife, ItemType::Axe, ItemType::Gun,
        ItemType::FirstAid, ItemType::Bandage
    };
    constexpr int lootTableSize = 5;

    for (size_t i = 0; i < itemSpawnPoints.size(); ++i)
    {
        ItemType lootType = lootTable[i % lootTableSize];
        m_itemPool.spawn(lootType, itemSpawnPoints[i], m_itemTypeRegistry);
    }

    std::cout << "Map regenerated with seed " << m_currentSeed << "!\n";
    std::cout << "======================================\n\n";

    m_dayTimer.reset();
}

void Game::run()
{
    
    if (!m_gameValid)
    {
        std::cout << "Game failed to initialize. Exiting..." << std::endl;
        std::cout << "Press Enter to close..." << std::endl;
        std::cin.get();
        return;
    }

    
    
    sf::Clock clock;
    sf::Time timeSinceLastUpdate = sf::Time::Zero;
    const float fps = 60.0f;
    sf::Time timePerFrame = sf::seconds(1.0f / fps);

    while (m_window.isOpen())
    {
        
        processEvents();

        
        timeSinceLastUpdate += clock.restart();

        
        while (timeSinceLastUpdate > timePerFrame)
        {
            timeSinceLastUpdate -= timePerFrame;
            m_input.update(m_window);              
            update(timePerFrame);          
        }

        
        render();
    }
}

void Game::processEvents()
{
    
    while (const std::optional newEvent = m_window.pollEvent())
    {
        
        if (newEvent->is<sf::Event::Closed>())
        {
            m_exitGame = true;
        }
        
        else if (const auto* mouseButtonPressed = newEvent->getIf<sf::Event::MouseButtonPressed>())
        {
            if (mouseButtonPressed->button == sf::Mouse::Button::Left)
            {
                m_mousePressed = true;
            }
        }
        
        else if (const auto* mouseButtonReleased = newEvent->getIf<sf::Event::MouseButtonReleased>())
        {
            if (mouseButtonReleased->button == sf::Mouse::Button::Left)
            {
                m_mousePressed = false;
            }
        }
        
        else if (const auto* keyPressed = newEvent->getIf<sf::Event::KeyPressed>())
        {
            
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
    
    if (m_exitGame)
    {
        m_window.close();
        return;
    }

    
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
    
    m_mainMenu.update(m_input);
}

void Game::updateSettings()
{
    
    m_settingsMenu.update(getMousePosition(), m_mousePressed);

    
    if (m_input.wasJustPressed(InputAction::Cancel) && !m_settingsMenu.isWaitingForKey())
    {
        onBackFromSettings();
    }
}

void Game::updatePlaying(sf::Time deltaTime)
{
    
    if (m_input.wasJustPressed(InputAction::Pause) || m_input.wasJustPressed(InputAction::Menu))
    {
        m_stateManager.pushState(GameState::Paused);  
        return;
    }

    
    if (m_input.wasMouseJustPressed() &&
        m_player.getCurrentState() != PlayerState::InventoryOpen)
    {
        float dmg = m_player.attack(getMouseWorldPosition());
        if (dmg > 0.f)
        {
            
            m_enemyManager.checkAttackHit(m_player.getPosition(),
                getMouseWorldPosition(), dmg,
                m_player.getEquippedWeapon());
        }
    }

    
    m_player.updateWithInput(deltaTime, m_input, getMouseWorldPosition());

    
    sf::FloatRect playerBounds = m_player.getBounds();
    auto collision = m_collisionManager.checkWorldCollisionDetailed(playerBounds, m_map.get());

    if (collision.collided)
    {
        
        sf::Vector2f correction = m_collisionManager.resolveCollision(collision);
        m_player.setPosition(m_player.getPosition() + correction);
    }

    
    if (m_mapGenerator.getObjectPlacer())
    {
        const auto& worldObjects = m_mapGenerator.getObjectPlacer()->getObjects();
        playerBounds = m_player.getBounds();
        auto objectCollision = m_collisionManager.checkCollisionWith(playerBounds, worldObjects);

        if (objectCollision.collided)
        {
            sf::Vector2f correction = m_collisionManager.resolveCollision(objectCollision);
            m_player.setPosition(m_player.getPosition() + correction);
        }
    }

    
    
    {
        sf::FloatRect playerBounds = m_player.getBounds();
        auto activeItems = m_itemPool.getActiveItems();

        for (WorldItem* item : activeItems)
        {
            if (!playerBounds.findIntersection(item->getBounds()).has_value())
                continue;

            
            const ItemTypeData* data = m_itemTypeRegistry.get(item->getType());
            if (!data)
                continue;

            
            if (m_player.collectItem(*data, m_itemPool.getAtlas()))
            {
                m_itemPool.despawn(item);
                std::cout << "Picked up: " << data->name << "\n";
            }
        }
    }

    
    
    
    
    
    

    m_dayTimer.update(deltaTime);

    
    m_enemyManager.updateAll(deltaTime, m_player.getPosition(),
        m_map.get(), m_collisionManager);

    updateCamera();
}

void Game::updatePaused()
{
    
    m_pauseMenu.update(m_input);
}

void Game::render()
{
    m_window.clear(sf::Color::Black);

    
    GameState currentState = m_stateManager.getCurrentState();
    GameState previousState = m_stateManager.getPreviousState();

    
    switch (currentState)
    {
    case GameState::MainMenu:
        
        m_mainMenu.render(m_window);
        break;

    case GameState::Settings:
        
        if (previousState == GameState::Paused || previousState == GameState::Playing)
        {
            m_window.setView(m_gameView);
            if (m_map) {
                m_map->render(m_window);

                
                if (m_mapGenerator.getObjectPlacer())
                {
                    m_mapGenerator.getObjectPlacer()->render(m_window, m_gameView);
                }
            }
            m_mapGenerator.getVoronoiDiagram()->renderDebug(m_window);
            m_player.render(m_window);
            m_enemyManager.renderAll(m_window);

        }
        m_window.setView(m_uiView);
        m_settingsMenu.render(m_window);  
        break;

    case GameState::Playing:
        
        m_window.setView(m_gameView);

        if (m_map) {
            
            m_map->render(m_window);

            
            if (m_mapGenerator.getObjectPlacer())
            {
                m_mapGenerator.getObjectPlacer()->render(m_window, m_gameView);
            }
        }

        
        m_itemPool.render(m_window, m_gameView);

        
        

        
        m_player.render(m_window);
        m_enemyManager.renderAll(m_window);

        
        m_window.setView(m_uiView);
        m_dayTimer.render(m_window);
        m_player.renderHUD(m_window);
        m_player.renderInventory(m_window);
        m_player.renderCursor(m_window);

        break;

    case GameState::Paused:
        
        m_window.setView(m_gameView);
        if (m_map)
        {
            m_map->render(m_window);

            if (m_mapGenerator.getObjectPlacer())
            {
                m_mapGenerator.getObjectPlacer()->render(m_window, m_gameView);
            }

            
            m_player.render(m_window);
            m_enemyManager.renderAll(m_window);
        }

        m_window.setView(m_uiView);
        m_dayTimer.render(m_window);
        m_pauseMenu.render(m_window);
        break;

    case GameState::GameOver:

        break;
    }

    m_window.display();
}

void Game::onStateEnter(GameState state)
{
    
    switch (state)
    {
    case GameState::MainMenu:
        m_mainMenu.setVisible(true);
        m_pauseMenu.setVisible(false);
        m_settingsMenu.setVisible(false);
        m_window.setMouseCursorVisible(true);  
        m_player.getCursor().setVisible(false); 
        break;

    case GameState::Settings:
        m_settingsMenu.setVisible(true);
        m_mainMenu.setVisible(false);
        m_pauseMenu.setVisible(false);
        m_window.setMouseCursorVisible(true);
        m_player.getCursor().setVisible(false);
        break;

    case GameState::Playing:
        
        m_mainMenu.setVisible(false);
        m_pauseMenu.setVisible(false);
        m_settingsMenu.setVisible(false);
        m_window.setMouseCursorVisible(false); 
        m_player.getCursor().setVisible(true);  
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

void Game::onStartGame()
{
    
    m_stateManager.changeState(GameState::Playing);
}

void Game::onOpenSettings()
{
    
    m_stateManager.pushState(GameState::Settings);
}

void Game::onResumeGame()
{
    
    m_stateManager.popState();
}

void Game::onQuitGame()
{
    m_exitGame = true;
}

void Game::onBackToMenu()
{
    
    m_stateManager.changeState(GameState::MainMenu);
}

void Game::onBackFromSettings()
{
    
    m_stateManager.popState();
}

void Game::onApplySettings()
{
    
    m_screenSettings.applySettings(m_window);

    
    const auto& res = m_screenSettings.getCurrentResolution();
    m_settingsMenu.updateLayout(res.width, res.height);

    std::cout << "Settings applied!" << std::endl;
}

void Game::updateCamera()
{
    
    sf::Vector2f playerPos = m_player.getPosition();

    
    sf::Vector2f clampedPos = clampCameraToMapBounds(playerPos);

    m_gameView.setCenter(clampedPos);
}

sf::Vector2f Game::clampCameraToMapBounds(const sf::Vector2f& targetPos)
{
    if (!m_map)
        return targetPos;

    sf::Vector2f viewSize = m_gameView.getSize();
    sf::Vector2f mapSize = m_map->getWorldSize();

    
    float halfWidth = viewSize.x / 2.f;
    float halfHeight = viewSize.y / 2.f;

    
    float clampedX = std::max(halfWidth, std::min(targetPos.x, mapSize.x - halfWidth));
    float clampedY = std::max(halfHeight, std::min(targetPos.y, mapSize.y - halfHeight));

    return sf::Vector2f(clampedX, clampedY);
}

sf::Vector2f Game::getMousePosition() const
{
    
    return sf::Vector2f(sf::Mouse::getPosition(m_window));
}

sf::Vector2f Game::getMouseWorldPosition() const
{
    
    sf::Vector2i mousePixel = sf::Mouse::getPosition(m_window);
    return m_window.mapPixelToCoords(mousePixel, m_gameView);
}
