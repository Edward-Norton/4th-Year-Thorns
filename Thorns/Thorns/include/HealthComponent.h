#ifndef HEALTH_COMPONENT_H
#define HEALTH_COMPONENT_H

#include "IUpdatable.h"
#include <algorithm>

/// <summary>
/// Stores and manages health data for any entity that owns it.
/// Just for health logic and nothing more to isoloate
/// </summary>
class HealthComponent : public IUpdatable
{
public:
    HealthComponent();
    ~HealthComponent() = default;

    // ========== Initialization ==========
    // Set max health and fill to full.
    void initialize(float maxHealth);

    // ========== IUpdatable ==========
    // To change it for like regen, dot, etc
    void update(sf::Time deltaTime) override;

    // ========== Damage / Healing ==========
    void takeDamage(float amount);
    void heal(float amount);
    void setHealth(float amount);
    void fillToMax();

    // ========== Queries ==========
    float getCurrentHealth() const { return m_currentHealth; }
    float getMaxHealth() const { return m_maxHealth; }

    // Returns 0.0 (dead) to 1.0 (full), gonna be used for HUD elements and UI
    float getHealthRatio() const;

    bool isDead() const { return m_currentHealth <= 0.f; }

private:
    float m_currentHealth;
    float m_maxHealth;
};

#endif