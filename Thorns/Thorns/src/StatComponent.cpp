#include "StatComponent.h"

StatComponent::StatComponent()
    : m_currentValue(100.f)
    , m_maxValue(100.f)
    , m_drainRate(0.f)
{
}

void StatComponent::initialize(float maxValue, float drainRate)
{
    m_maxValue = std::max(1.f, maxValue);
    m_currentValue = m_maxValue;
    m_drainRate = std::max(0.f, drainRate);
}

void StatComponent::update(sf::Time deltaTime)
{
    // Auto draint he value
    if (m_drainRate > 0.f)
        decrease(m_drainRate * deltaTime.asSeconds());
}

void StatComponent::decrease(float amount)
{
    m_currentValue = std::max(0.f, m_currentValue - amount);
}

void StatComponent::increase(float amount)
{
    m_currentValue = std::min(m_maxValue, m_currentValue + amount);
}

void StatComponent::setValue(float amount)
{
    m_currentValue = std::clamp(amount, 0.f, m_maxValue);
}

void StatComponent::fillToMax()
{
    m_currentValue = m_maxValue;
}

float StatComponent::getRatio() const
{
    if (m_maxValue <= 0.f)
        return 0.f;
    return m_currentValue / m_maxValue;
}