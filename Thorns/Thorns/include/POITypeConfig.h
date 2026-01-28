#ifndef POI_CONFIG_H
#define POI_CONFIG_H

#include <string>
#include <unordered_map>
#include <SFML/Graphics.hpp>
#include "PointOfInterest.h"

/// <summary>
/// POI TYPE CONFIG :
/// 
/// this is to stores static configuration for each POI type (paths, sizes, metadata)
/// 
/// Meant to find the resources need for the POIs
/// 
/// Need for creating a POI in mapGenerator
/// 
/// </summary>

// Just to hold the POI data 
struct POITypeConfig
{
    std::string name;
    std::string spritePath;
    std::string definitionsPath;
    std::string templatePath;
    sf::Vector2f size;
    bool autoSizeFromSprite;

    POITypeConfig()
        : size(0.f, 0.f)
        , autoSizeFromSprite(true)
    {
    }
};


// Collection of the Configs
// Just to set up the defaults for POI locations
class POIConfigRegistry
{
public:
    POIConfigRegistry();
    ~POIConfigRegistry() = default;

    // Set type to REG configs
    void registerPOIType(PointOfInterest::Type type, const POITypeConfig& config);

    // Get the POI type 
    const POITypeConfig* getConfig(PointOfInterest::Type type) const;

    // Just a check for POI confirmation
    bool hasConfig(PointOfInterest::Type type) const;

    // Load the defaults (NOTE: MAKE NEW ONES HERE)
    void loadDefaultConfigs();

private:
    // Types and setups
    std::unordered_map<PointOfInterest::Type, POITypeConfig> m_configs;

    // This is used to Auto-Size POIs, was annoying to set them up each time, so just gets the size from a file
    sf::Vector2f parseSizeFromDefinition(const std::string& definitionPath) const;
};

#endif