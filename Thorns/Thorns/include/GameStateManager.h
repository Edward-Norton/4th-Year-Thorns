/*
 * GameStateManager - Manages game state transitions with stack support
 *
 * Design Pattern: State Pattern with Pushdown Automaton (stack)
 *
 * Three types of state transitions:
 * 1. pushState() - Add new state on top (pause game, open settings)
 * 2. popState()  - Return to previous state (unpause, close settings)
 * 3. changeState() - Replace current state entirely (start game, quit to menu)
 *
 * Example state stack:
 * Playing -> Pause -> Settings
 * [Playing] [Playing, Paused] [Playing, Paused, Settings]
 *   ^push Pause      ^push Settings
 *
 * Pop twice to return to Playing
 */

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

    // ========== State Transitions ==========

    // Push a new state onto the stack (preserves current state)
    // Use for: Pause, opening menus during gameplay
    void pushState(GameState newState);

    // Pop back to the previous state on the stack
    // Use for: Unpausing, closing menus
    void popState();

    // Replace current state entirely (clears stack)
    // Use for: Starting game, quitting to main menu
    void changeState(GameState newState);

    // ========== State Queries ==========
    GameState getCurrentState() const { return m_currentState; }
    GameState getPreviousState() const { return m_previousState; }
    bool hasPreviousState() const { return !m_stateStack.empty(); }

    // ========== Optional Callbacks ==========
    // Register callbacks to be notified of state changes
    // Used for showing/hiding UI, cleanup, etc.
    void setOnStateEnter(std::function<void(GameState)> callback) { m_onStateEnter = callback; }
    void setOnStateExit(std::function<void(GameState)> callback) { m_onStateExit = callback; }

private:
    GameState m_currentState;              // Active state
    GameState m_previousState;             // For rendering context (e.g., show game under settings)
    std::stack<GameState> m_stateStack;    // Stack of paused states

    // Optional callbacks for external code to react to state changes
    std::function<void(GameState)> m_onStateEnter;
    std::function<void(GameState)> m_onStateExit;
};

#endif