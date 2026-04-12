#ifndef STAT_COMPONENT_H
#define STAT_COMPONENT_H

#include "IUpdatable.h"
#include <algorithm>
#include <string>

class StatComponent : public IUpdatable
{
public:
    StatComponent();
    ~StatComponent() = default;

    
    
    void initialize(float maxValue, float drainRate = 0.f);

    
    
    void update(sf::Time deltaTime) override;

    
    void decrease(float amount);
    void increase(float amount);
    void setValue(float amount);
    void fillToMax();

    
    
    void setDrainRate(float rate) { m_drainRate = rate; }
    float getDrainRate() const { return m_drainRate; }

    
    float getValue()    const { return m_currentValue; }
    float getMaxValue() const { return m_maxValue; }

    
    float getRatio() const;

    bool isEmpty() const { return m_currentValue <= 0.f; }

private:
    float m_currentValue;
    float m_maxValue;
    float m_drainRate;   
};

#endif