#include "POITypeConfig.h"
#include "AssetPaths.h"
#include <iostream>
#include <fstream>
#include <sstream>

POIConfigRegistry::POIConfigRegistry()
{
    loadDefaultConfigs();
}

void POIConfigRegistry::registerPOIType(PointOfInterest::Type type, const POITypeConfig& config)
{
    m_configs[type] = config;
}

const POITypeConfig* POIConfigRegistry::getConfig(PointOfInterest::Type type) const
{
    auto it = m_configs.find(type);
    if (it != m_configs.end())
        return &it->second;
    return nullptr;
}

bool POIConfigRegistry::hasConfig(PointOfInterest::Type type) const
{
    return m_configs.find(type) != m_configs.end();
}

// This is to provide a config for the POI template later. 
void POIConfigRegistry::loadDefaultConfigs()
{   
    // Hideout
    POITypeConfig hideoutConfig;
    hideoutConfig.name = "Hideout";
    hideoutConfig.spritePath = Assets::Textures::HIDEOUT_SPRITE;
    hideoutConfig.templatePath = Assets::Maps::HIDEOUT_TEMPLATE;
    hideoutConfig.autoSizeFromSprite = false;
    hideoutConfig.size = sf::Vector2f(481.0f, 419.0f);
    registerPOIType(PointOfInterest::Type::PlayerHideout, hideoutConfig);

    // Farm
    POITypeConfig farmConfig;
    farmConfig.name = "Farm";
    farmConfig.spritePath = Assets::Textures::FARM_SPRITE;
    farmConfig.definitionsPath = Assets::Data::FARM_DEFINITIONS;
    farmConfig.autoSizeFromSprite = true;
    registerPOIType(PointOfInterest::Type::Farm, farmConfig);

    // Village
    POITypeConfig villageConfig;
    villageConfig.name = "Village";
    villageConfig.spritePath = "";  
    villageConfig.templatePath = ""; 
    villageConfig.autoSizeFromSprite = false;
    villageConfig.size = sf::Vector2f(500.0f, 500.0f);
    registerPOIType(PointOfInterest::Type::Village, villageConfig);

    // This to try and auto size the tile areas needed based ont he above sizes. 
    // Found it annoying doing it each time and hopefully make implementation easier sob
    for (auto& [type, config] : m_configs)
    {
        if (config.autoSizeFromSprite && !config.definitionsPath.empty())
        {
            config.size = parseSizeFromDefinition(config.definitionsPath);
            std::cout << "Auto-sized " << config.name << ": "
                << config.size.x << "x" << config.size.y << "\n";
        }
    }
}

sf::Vector2f POIConfigRegistry::parseSizeFromDefinition(const std::string& definitionPath) const
{
    std::ifstream file(definitionPath);
    if (!file.is_open())
    {
        std::cerr << "Failed to open POI definition: " << definitionPath << "\n";
        return sf::Vector2f(500.f, 500.f);
    }

    std::string line;
    if (std::getline(file, line))
    {
        std::stringstream ss(line);
        std::string name;
        int x, y, width, height;
        char comma;

        std::getline(ss, name, ',');
        ss >> x >> comma >> y >> comma >> width >> comma >> height;

        return sf::Vector2f(static_cast<float>(width), static_cast<float>(height));
    }

    return sf::Vector2f(500.f, 500.f);
}