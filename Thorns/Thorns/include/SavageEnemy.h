#ifndef SAVAGE_ENEMY_H
#define SAVAGE_ENEMY_H

#include "SpriteComponent.h"
#include "IEnemy.h"
#include <SFML/System/Time.hpp>

class Map;


class SavageEnemy : public IEnemy
{
public:
    // AI behaviour states
    enum class AIState
    {
        Idle,   // No target visible, standing still
        Chase,  // Target in LOS, moving toward it
        Lost    // Target left LOS, holding last known position briefly
    };

    SavageEnemy();
    ~SavageEnemy() = default;

    // ========== Initialization ==========
    bool initialize(const std::string& atlasPath);

    // ========== IUpdatable ==========
    void update(sf::Time deltaTime) override;

    // Update with context the entity itself cannot own
    void updateWithContext(sf::Time deltaTime,
        const sf::Vector2f& playerPos,
        const Map* map);

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
    bool canSeePlayer() const { return m_aiState == AIState::Chase; }

private:
    void updateAI(const sf::Vector2f& playerPos, const Map* map);
    void updateMovement(sf::Time deltaTime);
    void faceDirection(const sf::Vector2f& direction);

    SpriteComponent m_sprite;
    bool            m_active;

    AIState         m_aiState;
    sf::Vector2f    m_lastKnownPlayerPos;

    // Lost-state timer: how long to linger at last known position
    float           m_lostTimer;
    static constexpr float LOST_DURATION = 2.5f;  // seconds before returning to Idle

    sf::Vector2f    m_velocity;

    // Tuning
    static constexpr float CHASE_SPEED = 120.f;
    static constexpr float LOS_RANGE = 550.f;
    static constexpr float ARRIVAL_RADIUS = 24.f;  // Stop moving when this close

    static constexpr float SPRITE_NORTH_OFFSET = -90.f;
};

#endif