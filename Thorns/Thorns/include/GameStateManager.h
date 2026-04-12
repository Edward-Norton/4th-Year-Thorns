

#ifndef GAME_STATE_MANAGER_HPP
#define GAME_STATE_MANAGER_HPP

#include <functional>
#include <stack>

enum class GameState
{
    MainMenu,
    Settings,
    Playing,
    Paused,
    GameOver
};

class GameStateManager
{
public:
    GameStateManager();
    ~GameStateManager() = default;

    

    
    
    void pushState(GameState newState);

    
    
    void popState();

    
    
    void changeState(GameState newState);

    
    GameState getCurrentState() const { return m_currentState; }
    GameState getPreviousState() const { return m_previousState; }
    bool hasPreviousState() const { return !m_stateStack.empty(); }

    
    
    
    void setOnStateEnter(std::function<void(GameState)> callback) { m_onStateEnter = callback; }
    void setOnStateExit(std::function<void(GameState)> callback) { m_onStateExit = callback; }

private:
    GameState m_currentState;              
    GameState m_previousState;             
    std::stack<GameState> m_stateStack;    

    
    std::function<void(GameState)> m_onStateEnter;
    std::function<void(GameState)> m_onStateExit;
};

#endif