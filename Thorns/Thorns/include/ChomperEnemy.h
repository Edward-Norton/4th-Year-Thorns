#ifndef CHOMPER_ENEMY_H
#define CHOMPER_ENEMY_H

#include "SpriteComponent.h"
#include "IEnemy.h"
#include <SFML/System/Time.hpp>

class Map;
class CollisionManager;

/// <summary>
/// Chomper enemy type.
/// 
/// AI States:
///   Idle   -> no target visible, stationary
///   Chase  -> target in LOS, walks toward it
///   Leap   -> within leap range, launches a fast lunge attack
///   Lost   -> target left LOS, holds last known position briefly
/// </summary>
class ChomperEnemy : public IEnemy
{
public:
    enum class AIState
    {
        Idle,
        Chase,
        Leap,   // Active lunge in flight
        Lost
    };

    ChomperEnemy();
    ~ChomperEnemy() = default;

    // ========== Initialization ==========
    // atlasPath: shared sprite sheet for the chomper
    bool initialize(const std::string& atlasPath);

    // ========== IUpdatable ==========
    void update(sf::Time deltaTime) override;

    // ========== IEnemy ==========
    void updateWithContext(sf::Time deltaTime,
        const sf::Vector2f& playerPos,
        const Map* map) override;

    // ========== Collision ==========
    void applyCollisionCorrection(const sf::Vector2f& correction);

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

    // ========== Queries ==========
    AIState getAIState() const { return m_aiState; }
    bool isLeaping()     const { return m_aiState == AIState::Leap; }

private:
    void updateAI(const sf::Vector2f& playerPos, const Map* map);
    void updateMovement(sf::Time deltaTime);
    void faceDirection(const sf::Vector2f& direction);
    void beginLeap(const sf::Vector2f& targetPos);

    SpriteComponent m_sprite;
    bool m_active;

    AIState m_aiState;
    sf::Vector2f m_lastKnownPlayerPos;
    sf::Vector2f m_velocity;

    // Lost-state timer
    float m_lostTimer;
    static constexpr float LOST_DURATION = 1.8f;

    // Leap state
    sf::Vector2f m_leapDirection;      // Normalised direction locked at leap start
    float        m_leapTimer;          // Elapsed time in current leap
    float        m_leapCooldownTimer;  // Time until next leap is allowed

    static constexpr float CHASE_SPEED = 90.f;
    static constexpr float LEAP_SPEED = 420.f;
    static constexpr float LEAP_RANGE = 220.f;  // Distance that triggers a leap
    static constexpr float LEAP_DURATION = 0.35f;  // Seconds leap lasts
    static constexpr float LEAP_COOLDOWN = 3.0f;   // Seconds between leaps
    static constexpr float LOS_RANGE = 500.f;
    static constexpr float ARRIVAL_RADIUS = 20.f;

    
    static constexpr float SPRITE_NORTH_OFFSET = 90.f;
};

#endif