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

    // ========== Initialization ==========
    bool initialize(float duration, const std::string& fontPath);

    // ========== IUpdatable ==========
    void update(sf::Time deltaTime) override;

    // ========== IRenderable ==========
    // Call with the UI view active
    void render(sf::RenderTarget& target) const override;

    // ========== Callback ==========
    // Fired once when the countdown reaches zero.
    void setOnExpired(std::function<void()> callback) { m_onExpired = callback; }

    // ========== Control ==========
    // Resets countdown to the original duration; called after map regeneration.
    void reset();

    // ========== Queries ==========
    float getRemainingSeconds() const { return m_remaining; }
    bool  hasExpired()          const { return m_expired; }

private:
    // Formats remaining seconds as "MM:SS" string.
    std::string formatTime() const;

    // ========== State ==========
    float m_duration;     // Original countdown duration in seconds
    float m_remaining;    // Seconds left
    bool  m_expired;      // True after callback has fired

    // ========== Rendering ==========
    sf::Font m_font;
    bool     m_fontLoaded;

    // ========== Callback ==========
    std::function<void()> m_onExpired;

    // ========== Layout (screen-space, UI view) ==========
    static constexpr float PANEL_WIDTH = 180.f;
    static constexpr float PANEL_HEIGHT = 60.f;
    static constexpr float PANEL_RIGHT_MARGIN = 20.f;  // Gap from right edge
    static constexpr float PANEL_TOP_MARGIN = 20.f;  // Gap from top edge
    static constexpr unsigned int FONT_SIZE_LABEL = 16u;
    static constexpr unsigned int FONT_SIZE_TIME = 28u;
};

#endif