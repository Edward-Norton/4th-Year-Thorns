#ifndef SAVAGE_ENEMY_H
#define SAVAGE_ENEMY_H

#include "SpriteComponent.h"
#include "IEnemy.h"
#include <SFML/System/Time.hpp>

class Map;

class SavageEnemy : public IEnemy
{
public:
    
    enum class AIState
    {
        Idle,   
        Chase,  
        Lost    
    };

    SavageEnemy();
    ~SavageEnemy() = default;

    
    bool initialize(const std::string& atlasPath);

    
    void update(sf::Time deltaTime) override;

    
    void updateWithContext(sf::Time deltaTime,
        const sf::Vector2f& playerPos,
        const Map* map);

    
    void render(sf::RenderTarget& target) const override;

    
    void setPosition(const sf::Vector2f& pos) override;
    sf::Vector2f getPosition() const override;

    
    sf::FloatRect getBounds() const override;

    
    bool isActive() const override { return m_active; }
    void setActive(bool active) override { m_active = active; }

    
    AIState getAIState() const { return m_aiState; }
    bool canSeePlayer() const { return m_aiState == AIState::Chase; }

    void takeDamage(float amount);

private:
    void updateAI(const sf::Vector2f& playerPos, const Map* map);
    void updateMovement(sf::Time deltaTime);
    void faceDirection(const sf::Vector2f& direction);

    SpriteComponent m_sprite;
    bool            m_active;

    AIState         m_aiState;
    sf::Vector2f    m_lastKnownPlayerPos;

    
    float           m_lostTimer;
    static constexpr float LOST_DURATION = 2.5f;  

    sf::Vector2f    m_velocity;

    
    static constexpr float CHASE_SPEED = 120.f;
    static constexpr float LOS_RANGE = 550.f;
    static constexpr float ARRIVAL_RADIUS = 24.f;  

    static constexpr float SPRITE_NORTH_OFFSET = 90.f;
};

#endif