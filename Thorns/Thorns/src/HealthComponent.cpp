#include "HealthComponent.h"
#include "StatComponent.h"

HealthComponent::HealthComponent()
    : m_currentHealth(100.f)
    , m_maxHealth(100.f)
{
}

void HealthComponent::initialize(float maxHealth)
{
    m_maxHealth = std::max(1.f, maxHealth); 
    m_currentHealth = m_maxHealth;
}

void HealthComponent::update(sf::Time deltaTime)
{
    
}

void HealthComponent::takeDamage(float amount)
{
    m_currentHealth = std::max(0.f, m_currentHealth - amount);
}

void HealthComponent::heal(float amount)
{
    m_currentHealth = std::min(m_maxHealth, m_currentHealth + amount);
}

void HealthComponent::setHealth(float amount)
{
    m_currentHealth = std::clamp(amount, 0.f, m_maxHealth);
}

void HealthComponent::fillToMax()
{
    m_currentHealth = m_maxHealth;
}

float HealthComponent::getHealthRatio() const
{
    if (m_maxHealth <= 0.f)
        return 0.f;
    return m_currentHealth / m_maxHealth;
}