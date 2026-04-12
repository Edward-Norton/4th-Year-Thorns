#include "ObjectPlacer.h"
#include "Map.h"
#include "MapTile.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include "AssetPaths.h"

ObjectPlacer::ObjectPlacer()
    : m_perlin(nullptr)
    , m_initialized(false)
    , m_atlasTextureLoaded(false)
{
}

bool ObjectPlacer::initialize(const std::string& atlasPath, 
                              const std::string& definitionsPath,
                              const std::string& collisionTmxPath)
{
    m_atlasPath = atlasPath;

    if (!m_sharedAtlasTexture.loadFromFile(atlasPath))
    {
        std::cerr << "ObjectPlacer: Failed to load texture atlas: " << atlasPath << "\n";
        m_atlasTextureLoaded = false;
        return false;
    }

    m_atlasTextureLoaded = true;
    std::cout << "ObjectPlacer: Loaded shared texture atlas: " << atlasPath << "\n";

    if (m_templateManager.loadTemplates(Assets::Maps::WORLD_OBJECTS_TEMPLATE)) {
        m_templatesLoaded = true;
    }
    else
        std::cerr << "ObjectPlacer: Failed to load world object templates for TMX collisions\n";

    
    if (!parseDefinitions(definitionsPath))
    {
        std::cerr << "ObjectPlacer: Failed to parse definitions from " << definitionsPath << "\n";
        return false;
    }

    m_initialized = true;
    std::cout << "ObjectPlacer initialized with " << m_definitions.size() << " object types\n";
    return true;
}

bool ObjectPlacer::parseDefinitions(const std::string& definitionsPath)
{
    std::ifstream file(definitionsPath);
    if (!file.is_open())
    {
        std::cerr << "Failed to open definitions file: " << definitionsPath << "\n";
        return false;
    }

    std::string line;
    while (std::getline(file, line))
    {
        
        if (line.empty())
            continue;

        
        std::stringstream ss(line);
        std::string name;
        int x, y, width, height;
        char comma;

        std::getline(ss, name, ',');
        ss >> x >> comma >> y >> comma >> width >> comma >> height;

        
        WorldObject::Type type;
        if (name == "SmallRoot")
            type = WorldObject::Type::SmallRoot;
        else if (name == "TreeTop_1")
            type = WorldObject::Type::TreeTop1;
        else if (name == "TreeTop_2")
            type = WorldObject::Type::TreeTop2;
        else if (name == "LargeRoot")
            type = WorldObject::Type::LargeRoot;
        else if (name == "SmallRoot_Basic")
            type = WorldObject::Type::SmallRootBasic;
        else
        {
            std::cout << "Unknown object type: " << name << ", skipping...\n";
            continue;
        }

        
        ObjectDefinition def;
        def.name = name;
        def.textureRect = sf::IntRect(sf::Vector2i(x, y), sf::Vector2i(width, height));

        
        float scale = 0.5f;
        def.size = sf::Vector2f(width * scale, height * scale);

        m_definitions[type] = def;

        std::cout << "Loaded object: " << name << " at (" << x << "," << y
            << ") size " << width << "x" << height << "\n";
    }

    return !m_definitions.empty();
}

static std::string getCollisionTemplateName(WorldObject::Type type)
{
    switch (type)
    {
    case WorldObject::Type::SmallRoot:      return "Tree_Stump_Small";
    case WorldObject::Type::LargeRoot:      return "Tree_Stump_Large";
    case WorldObject::Type::TreeTop1:       return "Tree_1";
    case WorldObject::Type::TreeTop2:       return "Tree_2";
    case WorldObject::Type::SmallRootBasic: return "Tree_Stump_Small";
    default:                                return "";
    }
}

void ObjectPlacer::generateObjects(Map* map, const PlacementSettings& settings, unsigned int seed)
{
    if (!m_initialized || !map)
    {
        std::cerr << "ObjectPlacer: Cannot generate - not initialized or null map\n";
        return;
    }

    
    clearObjects();

    
    m_perlin = std::make_unique<PerlinNoise>(seed);

    std::cout << "\n--- Phase 2: Perlin Noise Object Placement ---\n";
    std::cout << "Parameters:\n";
    std::cout << "  Frequency: " << settings.frequency << "\n";
    std::cout << "  Octaves: " << settings.octaves << "\n";
    std::cout << "  Threshold: " << settings.placementThreshold << "\n";
    std::cout << "  Object Type: " << static_cast<int>(settings.objectType) << "\n";

    
    const ObjectDefinition* def = getDefinition(settings.objectType);
    if (!def)
    {
        std::cerr << "No definition found for object type\n";
        return;
    }

    const sf::Vector2f tmxOrigin(0.f, 0.f);

    
    const int sampleStep = 2;  

    int width = map->getWidth();
    int height = map->getHeight();
    int objectsPlaced = 0;
    int tilesChecked = 0;

    
    for (int y = 0; y < height; y += sampleStep)
    {
        for (int x = 0; x < width; x += sampleStep)
        {
            ++tilesChecked;

            
            sf::Vector2f worldPos = map->tileToWorld(x, y);

            
            if (!isValidPlacement(worldPos, map, settings))
                continue;

            
            
            double noiseValue = m_perlin->octaveNoise2D(
                worldPos.x * settings.frequency,
                worldPos.y * settings.frequency,
                settings.octaves,
                settings.persistence
            );

            
            if (noiseValue > settings.placementThreshold)
            {
                
                auto object = std::make_unique<WorldObject>(settings.objectType, worldPos);

                
                if (object->loadSpriteFromTexture(m_sharedAtlasTexture, def->textureRect, def->size))
                {
                    if (m_templatesLoaded)
                    {
                        const auto* shapes = m_templateManager.getShapes(settings.objectType);
                        if (shapes) {
                            object->setCollisionShapes(shapes, tmxOrigin);
                            std::cout << "Object at (" << worldPos.x << "," << worldPos.y
                                << ") got " << shapes->size() << " shape(s)\n";
                        }

                    }
                    m_objects.push_back(std::move(object));
                    ++objectsPlaced;
                }
            }
        }
    }

    std::cout << "Object placement complete:\n";
    std::cout << "  Tiles checked: " << tilesChecked << "\n";
    std::cout << "  Objects placed: " << objectsPlaced << "\n";
    std::cout << "  Placement rate: " << (objectsPlaced * 100.0 / tilesChecked) << "%\n";
}

void ObjectPlacer::clearObjects()
{
    m_objects.clear();
    m_perlin.reset();
}

void ObjectPlacer::render(sf::RenderTarget& target, const sf::View& view) const
{
    if (m_objects.empty())
        return;

    
    sf::Vector2f viewCenter = view.getCenter();
    sf::Vector2f viewSize = view.getSize();
    sf::FloatRect viewBounds(
        sf::Vector2f(viewCenter.x - viewSize.x / 2.f, viewCenter.y - viewSize.y / 2.f),
        viewSize
    );

    
    const float padding = 256.f;
    viewBounds.position.x -= padding;
    viewBounds.position.y -= padding;
    viewBounds.size.x += padding * 2.f;
    viewBounds.size.y += padding * 2.f;

    
    int rendered = 0;
    for (const auto& object : m_objects)
    {
        sf::FloatRect objBounds = object->getBounds();

        
        if (viewBounds.findIntersection(objBounds).has_value())
        {
            object->render(target);
            ++rendered;
        }
    }
}

void ObjectPlacer::renderDebug(sf::RenderTarget& target, const sf::View& view) const
{
    
    for (const auto& object : m_objects)
    {
        sf::CircleShape circle(8.f);
        circle.setOrigin(sf::Vector2f(8.f, 8.f));
        circle.setPosition(object->getPosition());
        circle.setFillColor(sf::Color(255, 165, 0, 150));  
        circle.setOutlineColor(sf::Color::White);
        circle.setOutlineThickness(1.f);
        target.draw(circle);
    }
}

bool ObjectPlacer::isValidPlacement(const sf::Vector2f& worldPos, Map* map, const PlacementSettings& settings) const
{
    
    const MapTile* tile = map->getTileAtWorldPos(worldPos);
    if (!tile)
        return false;

    
    if (settings.respectPOIs && map->isInsidePOI(worldPos))
        return false;

    
    if (settings.grassOnly)
    {
        MapTile::TerrainType terrain = tile->getTerrainType();
        if (terrain != MapTile::TerrainType::Grass)
            return false;
    }

    
    if (!tile->isWalkable())
        return false;

    return true;
}

const ObjectPlacer::ObjectDefinition* ObjectPlacer::getDefinition(WorldObject::Type type) const
{
    auto it = m_definitions.find(type);
    if (it != m_definitions.end())
        return &it->second;
    return nullptr;
}

