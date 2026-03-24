#ifndef STAT_COMPONENT_H
#define STAT_COMPONENT_H

#include "IUpdatable.h"
#include <algorithm>
#include <string>

/// <summary>
/// So intead of making 3 classes for the hunger, stamina, water etc
/// Made a generic comp that will just represent a value without caring the fine details really
/// To be used with HUD so it can showcase the numbers under this class
/// </summary>
class StatComponent : public IUpdatable
{
public:
    StatComponent();
    ~StatComponent() = default;

    // ========== Initialization ==========
    // maxValue: upper limit. drainRate: units lost per second
    void initialize(float maxValue, float drainRate = 0.f);

    // ========== IUpdatable ==========
    // Applies auto-drain if drainRate > 0.
    void update(sf::Time deltaTime) override;

    // ========== Modification ==========
    void decrease(float amount);
    void increase(float amount);
    void setValue(float amount);
    void fillToMax();

    // ========== Drain Rate ==========
    // Adjust drain at runtime
    void setDrainRate(float rate) { m_drainRate = rate; }
    float getDrainRate() const { return m_drainRate; }

    // ========== Queries ==========
    float getValue()    const { return m_currentValue; }
    float getMaxValue() const { return m_maxValue; }

    // Linear representation of the stat for HUD bars
    float getRatio() const;

    bool isEmpty() const { return m_currentValue <= 0.f; }

private:
    float m_currentValue;
    float m_maxValue;
    float m_drainRate;   // Units per second - applied in update()
};

#endif