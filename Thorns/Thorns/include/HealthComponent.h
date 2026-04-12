#ifndef HEALTH_COMPONENT_H
#define HEALTH_COMPONENT_H

#include "IUpdatable.h"
#include <algorithm>

class HealthComponent : public IUpdatable
{
public:
    HealthComponent();
    ~HealthComponent() = default;

    
    
    void initialize(float maxHealth);

    
    
    void update(sf::Time deltaTime) override;

    
    void takeDamage(float amount);
    void heal(float amount);
    void setHealth(float amount);
    void fillToMax();

    
    float getCurrentHealth() const { return m_currentHealth; }
    float getMaxHealth() const { return m_maxHealth; }

    
    float getHealthRatio() const;

    bool isDead() const { return m_currentHealth <= 0.f; }

private:
    float m_currentHealth;
    float m_maxHealth;
};

#endif