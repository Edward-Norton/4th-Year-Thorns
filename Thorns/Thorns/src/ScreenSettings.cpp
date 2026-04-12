#include "ScreenSettings.h"
#include <algorithm>
#include <iostream>

ScreenSettings::ScreenSettings()
    : m_currentResolution(1920, 1080)
    , m_currentResolutionIndex(0)
    , m_isFullscreen(false)
{
    loadAvailableResolutions();
}

void ScreenSettings::loadAvailableResolutions()
{
    
    std::vector<Resolution> commonResolutions = {
        {1280, 720},
        {1600, 900},
        {1920, 1080},
        {2560, 1440},
        {3840, 2160}
    };

    
    std::vector<sf::VideoMode> modes = sf::VideoMode::getFullscreenModes();

    
    for (const auto& res : commonResolutions)
    {
        if (isResolutionSupported(res.width, res.height))
        {
            m_availableResolutions.push_back(res);
        }
    }

    
    if (m_availableResolutions.empty())
    {
        std::cerr << "Warning: No common resolutions supported, using desktop mode\n";
        sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
        m_availableResolutions.emplace_back(desktop.size.x, desktop.size.y);
    }

    
    for (size_t i = 0; i < m_availableResolutions.size(); ++i)
    {
        if (m_availableResolutions[i] == m_currentResolution)
        {
            m_currentResolutionIndex = static_cast<int>(i);
            return;
        }
    }

    
    m_currentResolutionIndex = 0;
    m_currentResolution = m_availableResolutions[0];
}

bool ScreenSettings::isResolutionSupported(unsigned int width, unsigned int height) const
{
    std::vector<sf::VideoMode> modes = sf::VideoMode::getFullscreenModes();

    for (const auto& mode : modes)
    {
        if (mode.size.x == width && mode.size.y == height)
        {
            return true;
        }
    }

    
    sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
    return (width <= desktop.size.x && height <= desktop.size.y);
}

void ScreenSettings::setResolution(int index)
{
    if (index >= 0 && index < static_cast<int>(m_availableResolutions.size()))
    {
        m_currentResolutionIndex = index;
        m_currentResolution = m_availableResolutions[index];
    }
}

void ScreenSettings::setResolution(unsigned int width, unsigned int height)
{
    
    for (int i = 0; i < static_cast<int>(m_availableResolutions.size()); ++i)
    {
        if (m_availableResolutions[i].width == width &&
            m_availableResolutions[i].height == height)
        {
            setResolution(i);
            return;
        }
    }

    std::cerr << "Warning: Resolution " << width << "x" << height << " not available\n";
}

void ScreenSettings::setFullscreen(bool fullscreen)
{
    m_isFullscreen = fullscreen;
}

void ScreenSettings::applySettings(sf::RenderWindow& window)
{
    
    window.close();

    
    sf::VideoMode videoMode = getCurrentVideoMode();

    
    sf::State windowState = m_isFullscreen ? sf::State::Fullscreen : sf::State::Windowed;

    
    window.create(videoMode, "THORNS", windowState);

    std::cout << "Applied settings: " << m_currentResolution.displayName
        << (m_isFullscreen ? " (Fullscreen)" : " (Windowed)") << std::endl;
}

sf::VideoMode ScreenSettings::getCurrentVideoMode() const
{
    return sf::VideoMode(
        sf::Vector2u(m_currentResolution.width, m_currentResolution.height),
        32U  
    );
}