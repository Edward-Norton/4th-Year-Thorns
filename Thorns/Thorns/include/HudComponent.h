#ifndef HUD_COMPONENT_H
#define HUD_COMPONENT_H

#include <SFML/Graphics.hpp>
#include <string>
#include "IRenderable.h"
#include "HealthComponent.h"
#include "StatComponent.h"

class HUDComponent : public IRenderable
{
public:
    explicit HUDComponent(const HealthComponent& health,
                          const StatComponent& stamina, 
                          const StatComponent& hunger, 
                          const StatComponent& water); 
    HUDComponent() = default;
    ~HUDComponent() = default;

    
    
    bool initialize(const std::string& fontPath);

    
    void render(sf::RenderTarget& target) const override;

private:
    
    void drawBar(sf::RenderTarget& target,
        float screenY,
        float ratio,
        const sf::Color& fillColor,
        const std::string& label,
        float current,
        float max) const;

    
    const HealthComponent& m_health;
    const StatComponent& m_stamina;
    const StatComponent& m_hunger;
    const StatComponent& m_water;

    
    sf::Font m_font;
    bool m_fontLoaded;

    
    
    static constexpr float BAR_X = 40.f;   
    static constexpr float BAR_Y = 20.f;   
    static constexpr float BAR_WIDTH = 200.f;  
    static constexpr float BAR_HEIGHT = 18.f;   
    static constexpr float LABEL_OFFSET_X = -4.f;   
    static constexpr float BAR_SPACING = 30.f;   

    
    static const sf::Color COLOR_BAR_BACKGROUND;
    static const sf::Color COLOR_BAR_OUTLINE;
    static const sf::Color COLOR_HEALTH_FILL;   
    static const sf::Color COLOR_STAMINA_FILL;  
    static const sf::Color COLOR_HUNGER_FILL;   
    static const sf::Color COLOR_WATER_FILL;    
    static const sf::Color COLOR_TEXT;
};

#endif