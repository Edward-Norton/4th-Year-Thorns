#include "GameStateManager.h"

GameStateManager::GameStateManager()
    : m_currentState(GameState::MainMenu)
    , m_previousState(GameState::MainMenu)
{
    // Start at main menu by default
}

void GameStateManager::pushState(GameState newState)
{
    /*
     * Push Transition: Pause current state and add new state on top
     *
     * Example: Playing -> push Paused
     * Stack: [Playing, Paused]
     *        Current = Paused, game logic freezes but renders in background
     */

     // Notify exit of current state
    if (m_onStateExit)
        m_onStateExit(m_currentState);

    // Save current state to stack
    m_stateStack.push(m_currentState);

    // Track state history for rendering context
    m_previousState = m_currentState;
    m_currentState = newState;

    // Notify entry to new state
    if (m_onStateEnter)
        m_onStateEnter(m_currentState);
}

void GameStateManager::popState()
{
    /*
     * Pop Transition: Return to previous state
     *
     * Example: [Playing, Paused] -> pop -> [Playing]
     *         Current = Playing, resume game
     */

     // Safety check: can't pop if stack is empty
    if (m_stateStack.empty())
        return;

    // Notify exit of current state
    if (m_onStateExit)
        m_onStateExit(m_currentState);

    // Track state history
    m_previousState = m_currentState;

    // Restore previous state from stack
    m_currentState = m_stateStack.top();
    m_stateStack.pop();

    // Notify entry to restored state
    if (m_onStateEnter)
        m_onStateEnter(m_currentState);
}

void GameStateManager::changeState(GameState newState)
{
    /*
     * Change Transition: Replace state entirely (no stack preservation)
     *
     * Example: MainMenu -> change to Playing
     * Stack cleared, can't return to MainMenu via pop
     * Use for: Starting game, quitting to menu
     */

     // Notify exit of current state
    if (m_onStateExit)
        m_onStateExit(m_currentState);

    // Track state history
    m_previousState = m_currentState;
    m_currentState = newState;

    // Clear the entire stack (fresh start)
    while (!m_stateStack.empty())
        m_stateStack.pop();

    // Notify entry to new state
    if (m_onStateEnter)
        m_onStateEnter(m_currentState);
}