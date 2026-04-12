#ifndef OBJECT_PLACER_HPP
#define OBJECT_PLACER_HPP

#include <memory>
#include <vector>
#include <unordered_map>
#include <string>
#include <SFML/Graphics.hpp>
#include "PerlinNoise.h"
#include "WorldObject.h"
#include "WorldObjectTemplate.h"

class Map;

class ObjectPlacer
{
public:
    
    struct PlacementSettings
    {
        
        double frequency = 0.1;          
        int octaves = 2;                 
        double persistence = 0.5;        

        
        double placementThreshold = 0.6; 
        double spacing = 32.0;           

        
        WorldObject::Type objectType = WorldObject::Type::SmallRoot;

        
        bool respectPOIs = true;         
        bool grassOnly = true;           
    };

    ObjectPlacer();
    ~ObjectPlacer() = default;

    

    
    bool initialize(const std::string& atlasPath,
        const std::string& definitionsPath,
        const std::string& collisionTmxPath = "");

    

    
    void generateObjects(Map* map, const PlacementSettings& settings, unsigned int seed);

    
    void clearObjects();

    

    
    void render(sf::RenderTarget& target, const sf::View& view) const;

    
    void renderDebug(sf::RenderTarget& target, const sf::View& view) const;

    

    int getObjectCount() const { return m_objects.size(); }
    const std::vector<std::unique_ptr<WorldObject>>& getObjects() const { return m_objects; }

private:
    

    
    struct ObjectDefinition
    {
        std::string name;
        sf::IntRect textureRect;  
        sf::Vector2f size;        
    };

    

    
    bool parseDefinitions(const std::string& definitionsPath);

    
    bool isValidPlacement(const sf::Vector2f& worldPos, Map* map, const PlacementSettings& settings) const;

    
    const ObjectDefinition* getDefinition(WorldObject::Type type) const;

    

    std::unique_ptr<PerlinNoise> m_perlin;
    std::vector<std::unique_ptr<WorldObject>> m_objects;

    
    sf::Texture m_sharedAtlasTexture;
    bool m_atlasTextureLoaded;

    
    std::string m_atlasPath;
    std::unordered_map<WorldObject::Type, ObjectDefinition> m_definitions;

    bool m_initialized;

    
    WorldObjectTemplateManager m_templateManager;
    bool m_templatesLoaded = false;
};

#endif

