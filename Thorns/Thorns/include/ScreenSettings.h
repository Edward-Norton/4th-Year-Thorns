#ifndef SCREEN_SETTINGS_H
#define SCREEN_SETTINGS_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

class ScreenSettings
{
public:
    
    struct Resolution
    {
        unsigned int width;
        unsigned int height;
        std::string displayName;

        Resolution(unsigned int w, unsigned int h)
            : width(w), height(h)
            , displayName(std::to_string(w) + "x" + std::to_string(h))
        {}

        bool operator==(const Resolution& other) const
        {
            return width == other.width && height == other.height;
        }
    };

    ScreenSettings();

    
    const std::vector<Resolution>& getAvailableResolutions() const { return m_availableResolutions; }
    const Resolution& getCurrentResolution() const { return m_currentResolution; }
    bool isFullscreen() const { return m_isFullscreen; }
    int getCurrentResolutionIndex() const { return m_currentResolutionIndex; }

    
    void setResolution(int index);
    void setResolution(unsigned int width, unsigned int height);
    void setFullscreen(bool fullscreen);
    void toggleFullscreen() { setFullscreen(!m_isFullscreen); }

    
    void applySettings(sf::RenderWindow& window);
    sf::VideoMode getCurrentVideoMode() const;

private:
    void loadAvailableResolutions();
    bool isResolutionSupported(unsigned int width, unsigned int height) const;

    std::vector<Resolution> m_availableResolutions;
    Resolution m_currentResolution;
    int m_currentResolutionIndex;
    bool m_isFullscreen;
};

#endif