#include "GameStateManager.h"

GameStateManager::GameStateManager()
    : m_currentState(GameState::MainMenu)
    , m_previousState(GameState::MainMenu)
{
    
}

void GameStateManager::pushState(GameState newState)
{
    

     
    if (m_onStateExit)
        m_onStateExit(m_currentState);

    
    m_stateStack.push(m_currentState);

    
    m_previousState = m_currentState;
    m_currentState = newState;

    
    if (m_onStateEnter)
        m_onStateEnter(m_currentState);
}

void GameStateManager::popState()
{
    

     
    if (m_stateStack.empty())
        return;

    
    if (m_onStateExit)
        m_onStateExit(m_currentState);

    
    m_previousState = m_currentState;

    
    m_currentState = m_stateStack.top();
    m_stateStack.pop();

    
    if (m_onStateEnter)
        m_onStateEnter(m_currentState);
}

void GameStateManager::changeState(GameState newState)
{
    

     
    if (m_onStateExit)
        m_onStateExit(m_currentState);

    
    m_previousState = m_currentState;
    m_currentState = newState;

    
    while (!m_stateStack.empty())
        m_stateStack.pop();

    
    if (m_onStateEnter)
        m_onStateEnter(m_currentState);
}