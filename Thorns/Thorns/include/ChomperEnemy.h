#ifndef CHOMPER_ENEMY_H
#define CHOMPER_ENEMY_H

#include "SpriteComponent.h"
#include "IEnemy.h"
#include <SFML/System/Time.hpp>

class Map;
class CollisionManager;

class ChomperEnemy : public IEnemy
{
public:
    enum class AIState
    {
        Idle,
        Chase,
        Leap,   
        Lost
    };

    ChomperEnemy();
    ~ChomperEnemy() = default;

    
    
    bool initialize(const std::string& atlasPath);

    
    void update(sf::Time deltaTime) override;

    
    void updateWithContext(sf::Time deltaTime,
        const sf::Vector2f& playerPos,
        const Map* map) override;

    
    void applyCollisionCorrection(const sf::Vector2f& correction);

    
    void render(sf::RenderTarget& target) const override;

    
    void setPosition(const sf::Vector2f& pos) override;
    sf::Vector2f getPosition() const override;

    
    sf::FloatRect getBounds() const override;

    
    bool isActive() const override { return m_active; }
    void setActive(bool active) override { m_active = active; }

    
    AIState getAIState() const { return m_aiState; }
    bool isLeaping()     const { return m_aiState == AIState::Leap; }

    void takeDamage(float amount);

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

    
    float m_lostTimer;
    static constexpr float LOST_DURATION = 1.8f;

    
    sf::Vector2f m_leapDirection;      
    float        m_leapTimer;          
    float        m_leapCooldownTimer;  

    static constexpr float CHASE_SPEED = 90.f;
    static constexpr float LEAP_SPEED = 420.f;
    static constexpr float LEAP_RANGE = 220.f;  
    static constexpr float LEAP_DURATION = 0.35f;  
    static constexpr float LEAP_COOLDOWN = 3.0f;   
    static constexpr float LOS_RANGE = 500.f;
    static constexpr float ARRIVAL_RADIUS = 20.f;

    
    static constexpr float SPRITE_NORTH_OFFSET = 90.f;
};

#endif