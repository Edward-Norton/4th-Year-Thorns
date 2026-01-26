#ifndef POI_CONFIG_H
#define POI_CONFIG_H

#include <string>
#include <unordered_map>
#include <SFML/Graphics.hpp>
#include "PointOfInterest.h"

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

class POIConfigRegistry
{
public:
    POIConfigRegistry();
    ~POIConfigRegistry() = default;

    void registerPOIType(PointOfInterest::Type type, const POITypeConfig& config);
    const POITypeConfig* getConfig(PointOfInterest::Type type) const;
    bool hasConfig(PointOfInterest::Type type) const;

    void loadDefaultConfigs();

private:
    std::unordered_map<PointOfInterest::Type, POITypeConfig> m_configs;

    sf::Vector2f parseSizeFromDefinition(const std::string& definitionPath) const;
};

#endif