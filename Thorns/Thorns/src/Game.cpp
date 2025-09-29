#include "Game.h"
#include <iostream>

Game::Game() :
    window{ sf::VideoMode{ sf::Vector2u{1920U, 1080U}, 32U }, "SFML Game 3.0" },
    m_gameValid(false)
{
    m_gameValid = validateEntities();
}

Game::~Game()
{
}

bool Game::validateEntities()
{
    if (!m_player.isValid())
    {
        std::cout << "Failed to load player!" << std::endl;
        return false;
    }
    if (!m_enemy.isValid())
    {
        std::cout << "Failed to load enemy!" << std::endl;
        return false;
    }

    std::cout << "All entities loaded successfully!" << std::endl;
    return true;
}

void Game::run()
{
    if (!m_gameValid)
    {
        std::cout << "Game failed to initialize. Exiting..." << std::endl;
        return;
    }

    sf::Clock clock;
    sf::Time timeSinceLastUpdate = sf::Time::Zero;
    const float fps{ 60.0f };
    sf::Time timePerFrame = sf::seconds(1.0f / fps);

    while (window.isOpen())
    {
        processEvents();
        timeSinceLastUpdate += clock.restart();
        while (timeSinceLastUpdate > timePerFrame)
        {
            timeSinceLastUpdate -= timePerFrame;
            processEvents();
            update(timePerFrame);
        }
        render();
    }
}

void Game::processEvents()
{
    while (const std::optional newEvent = window.pollEvent())
    {
        if (newEvent->is<sf::Event::Closed>())
        {
            exitGame = true;
        }
        if (newEvent->is<sf::Event::KeyPressed>())
        {
            processKeys(newEvent);
        }
    }
}

void Game::processKeys(const std::optional<sf::Event> t_event)
{
    const sf::Event::KeyPressed* newKeypress = t_event->getIf<sf::Event::KeyPressed>();
    if (sf::Keyboard::Key::Escape == newKeypress->code)
    {
        exitGame = true;
    }
}

void Game::checkKeyboardState()
{
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape))
    {
        exitGame = true;
    }
}

void Game::update(sf::Time t_deltaTime)
{
    checkKeyboardState();
    if (exitGame)
    {
        window.close();
    }

    m_player.update(t_deltaTime);
    m_enemy.update(t_deltaTime);
}

void Game::render()
{
    window.clear(sf::Color::Black);

    m_player.render(window);
    m_enemy.render(window);

    window.display();
}