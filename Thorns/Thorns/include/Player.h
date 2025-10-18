#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "SpriteComponent.h"
#include "CursorComponent.h"
#include "IGameEntity.h"
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>
#include "Inventory.h"

class InputController;

// ========== PLAYER MOVEMENT STATES ==========
enum class PlayerState
{
    Idle,   // Not moving
    Walk,   // Normal movement
    Sprint,  // Fast movement
    InventoryOpen // Seperate state since I want player not move when its open
};


class Player : public IGameEntity
{
public:
    Player();
    ~Player() = default;

    // ========== Initialization ==========
    bool initialize(const std::string& texturePath);

    // ========== IUpdatable ==========
    void update(sf::Time deltaTime) override;

    // ========== Input Updates ==========
    // Updates input state AND game logic in one call
    void updateWithInput(sf::Time deltaTime, const InputController& input, const sf::Vector2f& mousePosition);

    // ========== IRenderable ==========
    void render(sf::RenderTarget& target) const override;

    // ========== IPositionable ==========
    void setPosition(const sf::Vector2f& pos) override;
    sf::Vector2f getPosition() const override;

    // ========== ICollidable ==========
    sf::FloatRect getBounds() const override;

    // ========== IGameEntity ==========
    bool isActive() const override { return m_active; }
    void setActive(bool active) override { m_active = active; }

    // ========== State ==========
    bool isValid() const { return m_sprite.isValid(); }
    PlayerState getCurrentState() const { return m_currentState; }

    // ========== Cursor ==========
    CursorComponent& getCursor() { return m_cursor; }
    const CursorComponent& getCursor() const { return m_cursor; }

    // ========== Inventory ==========
    Inventory& getInventory() { return m_inventory; }
    const Inventory& getInventory() const { return m_inventory; }

private:
    // ========== State Machine ==========
    void updateState();
    void changeState(PlayerState newState);

    // ========== Movement ==========
    void updateMovement(sf::Time deltaTime);
    void updateRotation();
    void updateCursor();
    sf::Vector2f calculateMovementInput() const;
    float getCurrentSpeed() const;

    // ========== Components ==========
    SpriteComponent m_sprite;
    CursorComponent m_cursor;
    Inventory m_inventory;

    // ========== State ==========
    PlayerState m_currentState;
    bool m_active;

    // ========== Input References ==========
    const InputController* m_inputController;  // Non-owning pointer
    sf::Vector2f m_mousePosition;

    // ========== Physics ==========
    sf::Vector2f m_velocity;
    sf::Angle m_targetRotation;
    sf::Angle m_currentRotation;

    // ========== Movement Parameters ==========
    static constexpr float WALK_SPEED = 150.0f;
    static constexpr float SPRINT_SPEED = 225.0f;
    static constexpr float ROTATION_SPEED = 360.0f;  // Degrees per second
    static constexpr float ACCELERATION = 1200.0f;   // How fast we reach target speed
    static constexpr float DECELERATION = 1800.0f;   // How fast we stop (faster than acceleration)
    static constexpr float FRICTION = 0.85f;         // Velocity dampening each frame (0-1)
};

#endif