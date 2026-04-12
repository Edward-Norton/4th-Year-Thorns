#ifndef DAY_TIMER_COMPONENT_H
#define DAY_TIMER_COMPONENT_H

#include <SFML/Graphics.hpp>
#include <functional>
#include <string>
#include "IUpdatable.h"
#include "IRenderable.h"

class DayTimerComponent : public IUpdatable, public IRenderable
{
public:
    DayTimerComponent();
    ~DayTimerComponent() = default;

    
    bool initialize(float duration, const std::string& fontPath);

    
    void update(sf::Time deltaTime) override;

    
    
    void render(sf::RenderTarget& target) const override;

    
    
    void setOnExpired(std::function<void()> callback) { m_onExpired = callback; }

    
    
    void reset();

    
    float getRemainingSeconds() const { return m_remaining; }
    bool  hasExpired()          const { return m_expired; }

private:
    
    std::string formatTime() const;

    
    float m_duration;     
    float m_remaining;    
    bool  m_expired;      

    
    sf::Font m_font;
    bool     m_fontLoaded;

    
    std::function<void()> m_onExpired;

    
    static constexpr float PANEL_WIDTH = 180.f;
    static constexpr float PANEL_HEIGHT = 60.f;
    static constexpr float PANEL_RIGHT_MARGIN = 20.f;  
    static constexpr float PANEL_TOP_MARGIN = 20.f;  
    static constexpr unsigned int FONT_SIZE_LABEL = 16u;
    static constexpr unsigned int FONT_SIZE_TIME = 28u;
};

#endif