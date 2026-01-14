#include "ObjectPlacer.h"
#include "Map.h"
#include "MapTile.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>

ObjectPlacer::ObjectPlacer()
    : m_perlin(nullptr)
    , m_initialized(false)
{
}

bool ObjectPlacer::initialize(const std::string& atlasPath, const std::string& definitionsPath)
{
    m_atlasPath = atlasPath;

    // Parse object definitions
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
        // Skip empty lines
        if (line.empty())
            continue;

        // Parse format: Name,X,Y,Width,Height
        std::stringstream ss(line);
        std::string name;
        int x, y, width, height;
        char comma;

        std::getline(ss, name, ',');
        ss >> x >> comma >> y >> comma >> width >> comma >> height;

        // Map name to type
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

        // Store definition
        ObjectDefinition def;
        def.name = name;
        def.textureRect = sf::IntRect(sf::Vector2i(x, y), sf::Vector2i(width, height));

        // Scale down large objects to fit in world better
        // Adjust these scale factors as needed
        float scale = 0.5f;  // 50% of original size
        def.size = sf::Vector2f(width * scale, height * scale);

        m_definitions[type] = def;

        std::cout << "Loaded object: " << name << " at (" << x << "," << y
            << ") size " << width << "x" << height << "\n";
    }

    return !m_definitions.empty();
}

void ObjectPlacer::generateObjects(Map* map, const PlacementSettings& settings, unsigned int seed)
{
    if (!m_initialized || !map)
    {
        std::cerr << "ObjectPlacer: Cannot generate - not initialized or null map\n";
        return;
    }

    // Clear existing objects
    clearObjects();

    // Initialize Perlin noise with seed
    m_perlin = std::make_unique<PerlinNoise>(seed + 1000); // Offset seed to differ from Voronoi

    std::cout << "\n--- Phase 2: Perlin Noise Object Placement ---\n";
    std::cout << "Parameters:\n";
    std::cout << "  Frequency: " << settings.frequency << "\n";
    std::cout << "  Octaves: " << settings.octaves << "\n";
    std::cout << "  Threshold: " << settings.placementThreshold << "\n";
    std::cout << "  Object Type: " << static_cast<int>(settings.objectType) << "\n";

    // Get object definition
    const ObjectDefinition* def = getDefinition(settings.objectType);
    if (!def)
    {
        std::cerr << "No definition found for object type\n";
        return;
    }

    // Sample every Nth tile for performance (adjust as needed)
    const int sampleStep = 2;  // Check every 2nd tile

    int width = map->getWidth();
    int height = map->getHeight();
    int objectsPlaced = 0;
    int tilesChecked = 0;

    // Iterate through map tiles
    for (int y = 0; y < height; y += sampleStep)
    {
        for (int x = 0; x < width; x += sampleStep)
        {
            ++tilesChecked;

            // Get world position (center of tile)
            sf::Vector2f worldPos = map->tileToWorld(x, y);

            // Check if this location is valid for placement
            if (!isValidPlacement(worldPos, map, settings))
                continue;

            // Generate noise value at this position
            // Apply frequency to zoom in/out
            double noiseValue = m_perlin->octaveNoise2D(
                worldPos.x * settings.frequency,
                worldPos.y * settings.frequency,
                settings.octaves,
                settings.persistence
            );

            // Check threshold
            if (noiseValue > settings.placementThreshold)
            {
                // Create object
                auto object = std::make_unique<WorldObject>(settings.objectType, worldPos);

                // Load sprite from atlas
                if (object->loadSprite(m_atlasPath, def->textureRect, def->size))
                {
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

    // Get view frustum for culling
    sf::Vector2f viewCenter = view.getCenter();
    sf::Vector2f viewSize = view.getSize();
    sf::FloatRect viewBounds(
        sf::Vector2f(viewCenter.x - viewSize.x / 2.f, viewCenter.y - viewSize.y / 2.f),
        viewSize
    );

    // Add padding for partially visible objects
    const float padding = 256.f;
    viewBounds.position.x -= padding;
    viewBounds.position.y -= padding;
    viewBounds.size.x += padding * 2.f;
    viewBounds.size.y += padding * 2.f;

    // Render only visible objects
    int rendered = 0;
    for (const auto& object : m_objects)
    {
        sf::FloatRect objBounds = object->getBounds();

        // Frustum culling
        if (viewBounds.findIntersection(objBounds).has_value())
        {
            object->render(target);
            ++rendered;
        }
    }
}

void ObjectPlacer::renderDebug(sf::RenderTarget& target, const sf::View& view) const
{
    // Draw circles at object positions for debugging
    for (const auto& object : m_objects)
    {
        sf::CircleShape circle(8.f);
        circle.setOrigin(sf::Vector2f(8.f, 8.f));
        circle.setPosition(object->getPosition());
        circle.setFillColor(sf::Color(255, 165, 0, 150));  // Orange, semi-transparent
        circle.setOutlineColor(sf::Color::White);
        circle.setOutlineThickness(1.f);
        target.draw(circle);
    }
}

bool ObjectPlacer::isValidPlacement(const sf::Vector2f& worldPos, Map* map, const PlacementSettings& settings) const
{
    // Get tile at this position
    const MapTile* tile = map->getTileAtWorldPos(worldPos);
    if (!tile)
        return false;

    // Check if in POI area
    if (settings.respectPOIs && map->isInsidePOI(worldPos))
        return false;

    // Check terrain type
    if (settings.grassOnly)
    {
        MapTile::TerrainType terrain = tile->getTerrainType();
        if (terrain != MapTile::TerrainType::Grass)
            return false;
    }

    // Check if tile is walkable (don't place on walls, water, etc.)
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

// ========================================================================================================
// IMPLEMENTATION NOTES
// ========================================================================================================
/*
 * PERLIN NOISE OBJECT PLACEMENT STRATEGY:
 *
 * 1. Sample Grid:
 *    - Check every 2nd tile (sampleStep = 2) for performance
 *    - Can adjust based on object density needs
 *
 * 2. Noise Evaluation:
 *    - Calculate noise at tile center position
 *    - Multiply by frequency to control pattern size
 *    - Use octaves for detail variation
 *
 * 3. Threshold Check:
 *    - Only place object if noise > threshold
 *    - Higher threshold = sparser placement
 *    - Lower threshold = denser placement
 *
 * 4. Validation:
 *    - Respect POI exclusion zones
 *    - Only place on specific terrain types
 *    - Ensure tile is walkable
 *
 * PARAMETER TUNING GUIDE:
 *
 * For forest trees:
 * - frequency: 0.05 - 0.08 (large organic clusters)
 * - octaves: 2-3 (some variation)
 * - threshold: 0.6-0.7 (moderate density)
 *
 * For grass/flowers:
 * - frequency: 0.2 - 0.5 (small scattered patches)
 * - octaves: 1-2 (less variation needed)
 * - threshold: 0.5-0.6 (higher density)
 *
 * For rocks/boulders:
 * - frequency: 0.1 - 0.15 (medium clusters)
 * - octaves: 2 (some detail)
 * - threshold: 0.7-0.8 (very sparse)
 *
 * PERFORMANCE CONSIDERATIONS:
 *
 * - sampleStep reduces tiles checked (step=2 means 75% reduction)
 * - Octave noise takes ~2-4x longer than single noise
 * - Frustum culling prevents rendering off-screen objects
 * - All objects share same texture atlas (single texture bind)
 *
 * ALTERNATIVE APPROACHES:
 *
 * If Perlin noise gives too "blobby" patterns, consider:
 * - Poisson disc sampling (more even distribution)
 * - Voronoi-based placement (one object per region)
 * - Jittered grid (regular with random offset)
 * - Hybrid: Perlin for density, Poisson for exact positions
 */