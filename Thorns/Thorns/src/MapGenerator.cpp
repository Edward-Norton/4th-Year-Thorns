#include "MapGenerator.h"
#include "PointOfInterest.h"
#include <iostream>
#include <chrono>
#include "AssetPaths.h"

MapGenerator::MapGenerator()
    : m_voronoi(std::make_unique<VoronoiDiagram>())
    , m_objectPlacer(std::make_unique<ObjectPlacer>())
{
    m_poiTemplates.loadTemplate("hideout", "ASSETS/MAPS/POI_Templates/hideout.tmx");
}

std::unique_ptr<Map> MapGenerator::generate(const GenerationSettings& settings)
{
    // Step 2: Generate the base map (pn step 1 in game.cpp)
    std::cout << "\n=== Starting Map Generation ===\n";

    auto startTotal = std::chrono::high_resolution_clock::now();

    // Step 2.1: All tiles are UNKNOWN initally and use the default params while also alocating memory for map
    auto map = std::make_unique<Map>();
    map->initialize(settings.mapWidth, settings.mapHeight, settings.tileSize);

    // Step 2.2: Load the terrain atlas (to be updated with new types)
    if (!map->loadTerrainAtlas(Assets::Textures::TERRAIN_ATLAS)) {
        std::cerr << "Failed to load the Map terrain atlas, defaulted to debug rendering\n";
        map->setDebugMode(true);
    }

    // Setup static POIs (hideout at the center)
    // Step 2.3: Where the hideout needs to be based on map size
    setupHideoutPOI(map.get());
    map->markPOITiles();
    

    // Run generation phases
    // ========== VORONOI DIAGRAMS ==========
    //Step 3: Seeds Voronoi
    std::cout << "\n--- Phase 1: Voronoi Diagram ---\n";
    phase1_Voronoi(map.get(), settings);

    // Step 3.1: Spawn POIs at Voronoi sites
    std::cout << "\n--- Spawning POIs at Voronoi sites ---\n";
    spawnPOIsAtSites(map.get(), settings);
    map->markPOITiles();

    // ========== PERLIN NOISE ==========
    std::cout << "\n--- Phase 2: Perlin Noise ---\n";
    if (settings.enableObjectPlacement)
    {
        phase2_PerlinObjects(map.get(), settings);
    }
    else
    {
        std::cout << "\n--- Phase 2: Perlin Noise (SKIPPED) ---\n";
    }

    std::cout << "\n--- Phase 3: Cellular Automata ---\n";

    std::cout << "\n--- Phase 4: Connectivity Check ---\n";

    std::cout << "\n=== Map Generation Complete ===\n\n";

    auto endTotal = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double, std::milli> elapsed = endTotal - startTotal;

    
    std::cout <<"\n====GENERATION TIME===\n" << "mapGeneration took " << elapsed.count() << " ms\n"
        << "With a map size of " << std::to_string(settings.mapHeight) << "x" << std::to_string(settings.mapWidth) << "\n";


    return map;
}

void MapGenerator::regenerate(Map* map, const GenerationSettings& settings)
{
    if (!map)
    {
        std::cerr << "MapGenerator::regenerate() - null map pointer!\n";
        return;
    }

    std::cout << "\n=== Regenerating Existing Map ===\n";

    // Reset map data (clears tiles and POIs, keeps memory allocated)
    map->reset();

    // Setup static POIs (hideout at the center)
    setupHideoutPOI(map);
    map->markPOITiles();

    // Run generation phases
    // ========== VORONOI DIAGRAMS ==========
    std::cout << "\n--- Phase 1: Voronoi Diagram ---\n";
    phase1_Voronoi(map, settings);

    // Spawn POIs at Voronoi sites
    std::cout << "\n--- Spawning POIs at Voronoi sites ---\n";
    spawnPOIsAtSites(map, settings);
    map->markPOITiles();

    // ========== PERLIN NOISE ==========
    std::cout << "\n--- Phase 2: Perlin Noise ---\n";
    if (settings.enableObjectPlacement)
    {
        phase2_PerlinObjects(map, settings);
    }

    std::cout << "\n--- Phase 3: Cellular Automata ---\n";

    std::cout << "\n--- Phase 4: Connectivity Check ---\n";

    std::cout << "\n=== Map Regeneration Complete ===\n\n";
}


// ========================================================================================================
// Voronoi + Terrain Assignment in ONE LOOP
// - Was using multiple for loops, so one after another to accomplish result, using 1 for multi use instead. 
// - From 3 for loops to 1
// ========================================================================================================
void MapGenerator::phase1_Voronoi(Map* map, const GenerationSettings& settings)
{
    // Step 1: Generate Voronoi sites only (no tile iteration yet)
    std::mt19937 rng(settings.seed == 0 ? std::random_device{}() : settings.seed);
    sf::Vector2f worldSize = map->getWorldSize();
    
    m_voronoi->generateSitesPoisson(map, settings.voronoiSites, m_hideoutPosition,
        settings.minSiteDistance, rng);

    // Step 2: Build spatial grid for fast nearest-neighbor queries
    const auto& sites = m_voronoi->getSites();
    std::cout << "Building spatial grid for " << sites.size() << " sites...\n";

    m_voronoi->buildSpatialGrid(worldSize.x, worldSize.y, settings.minSiteDistance);

    // Adding each tile to the spatial grid for hash lookups
    for (size_t i = 0; i < sites.size(); ++i)
    {
        m_voronoi->addSiteToGrid(static_cast<int>(i), sites[i].position);
    }

    // Step 3: Single loop - assign regions AND set terrain together
    int width = map->getWidth();
    int height = map->getHeight();
    int tilesProcessed = 0;

    // ========== Single Loop: Do everything in one pass (try to reduce n-notation hopefully)  ==========
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            MapTile* tile = map->getTile(x, y);
            if (!tile)
                continue;

            // Skip POI tiles (hideout area)
            if (tile->getTerrainType() == MapTile::TerrainType::POI_Collision)
                continue;

            // Get tile world position
            sf::Vector2f tilePos = map->tileToWorld(x, y);

            // Find nearest Voronoi site using spatial grid (O(k) instead of O(n))
            std::vector<int> nearbySites = m_voronoi->getNearbySites(tilePos);

            int closestRegion = -1;
            float closestDistSq = std::numeric_limits<float>::max();

            // Check only nearby sites (spatial optimization)
            for (int siteIdx : nearbySites)
            {
                const VoronoiSite& site = sites[siteIdx];
                float dx = tilePos.x - site.position.x;
                float dy = tilePos.y - site.position.y;
                float distSq = dx * dx + dy * dy;

                if (distSq < closestDistSq)
                {
                    closestDistSq = distSq;
                    closestRegion = site.regionId;
                }
            }

            // Assign Voronoi region
            tile->setVoronoiRegion(closestRegion);

            // Set terrain type
            tile->setTerrainType(MapTile::TerrainType::Grass);
            tile->setWalkable(true);

            ++tilesProcessed;
        }
    }

    std::cout << "Voronoi assignment complete: " << tilesProcessed << " tiles processed\n";

    // Debug: Count tiles per region
    std::unordered_map<int, int> regionCounts;
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            const MapTile* tile = map->getTile(x, y);
            if (tile && tile->getVoronoiRegion() != -1)
            {
                regionCounts[tile->getVoronoiRegion()]++;
            }
        }
    }

    std::cout << "Tiles per region:\n";
    for (const auto& [regionId, count] : regionCounts)
    {
        std::cout << "  Region " << regionId << ": " << count << " tiles\n";
    }
}

// ========================================================================================================
// PHASE 2: PERLIN NOISE OBJECT PLACEMENT
// ========================================================================================================

void MapGenerator::phase2_PerlinObjects(Map* map, const GenerationSettings& settings)
{
    std::cout << "\n--- Phase 2: Perlin Noise Object Placement ---\n";

    // Prevent assets being initalized again upon regeneration. 
    if (!m_perlinAssetsInit)
    {
        if (!m_objectPlacer->initialize(
            Assets::Textures::FOREST_ATLAS,
            Assets::Data::FOREST_ATLAS_DEFINITIONS))
        {
            std::cerr << "Failed to initialize ObjectPlacer!\n";
            return;
        }
        m_perlinAssetsInit = true;
    }

    // Configure placement settings
    ObjectPlacer::PlacementSettings placementSettings;
    placementSettings.frequency = settings.objectFrequency;
    placementSettings.octaves = settings.objectOctaves;
    placementSettings.persistence = 0.5;
    placementSettings.placementThreshold = settings.objectThreshold;
    placementSettings.objectType = WorldObject::Type::SmallRoot;  // Testing with SmallRoot
    placementSettings.respectPOIs = true;  // Don't place in POI areas
    placementSettings.grassOnly = true;    // Only on grass terrain

    // Generate objects
    m_objectPlacer->generateObjects(map, placementSettings, settings.seed);

    std::cout << "Phase 2 complete: " << m_objectPlacer->getObjectCount() << " objects placed\n";
}

// ========================================================================================================
// POI MANAGEMENT
// ========================================================================================================
void MapGenerator::setupHideoutPOI(Map* map)
{
    sf::Vector2f worldSize = map->getWorldSize();
    m_hideoutPosition = sf::Vector2f(worldSize.x / 2.f, worldSize.y / 2.f);

    // Player Hideout at center
    auto hideout = std::make_unique<PointOfInterest>(
        "Player Hideout",
        PointOfInterest::Type::PlayerHideout,
        m_hideoutPosition,
        sf::Vector2f(481.0f, 419.0f)
    );

    // Load hideout sprite
    if (!hideout->loadSprite(Assets::Textures::HIDEOUT_SPRITE))
    {
        std::cerr << "Failed to load hideout sprite!\n";
    }

    m_poiTemplates.applyTemplateCollision(hideout.get(), "hideout");

    std::cout << "Placed hideout at map center: (" 
              << m_hideoutPosition.x << ", " << m_hideoutPosition.y << ")\n";

    map->addPOI(std::move(hideout));
}

void MapGenerator::spawnPOIsAtSites(Map* map, const GenerationSettings& settings)
{
    const auto& sites = m_voronoi->getSites();

    if (sites.empty())
    {
        std::cerr << "No Voronoi sites available for POI placement\n";
        return;
    }

    int totalPOIs = settings.numVillages + settings.numFarms;

    if (totalPOIs > static_cast<int>(sites.size()))
    {
        std::cerr << "Warning: Requested " << totalPOIs << " POIs but only "
            << sites.size() << " sites available!\n";
        totalPOIs = static_cast<int>(sites.size());
    }

    std::mt19937 rng(settings.seed == 0 ? std::random_device{}() : settings.seed);
    std::uniform_int_distribution<int> siteDist(0, static_cast<int>(sites.size()) - 1);

    int villagesLeft = settings.numVillages;
    int farmsLeft = settings.numFarms;
    std::vector<bool> usedSites(sites.size(), false);

    sf::Vector2f worldSize = map->getWorldSize();

    int poisSpawned = 0;
    int attempts = 0;
    const int maxAttempts = totalPOIs * 20;

    while (poisSpawned < totalPOIs && attempts < maxAttempts)
    {
        int siteIndex = siteDist(rng);

        if (usedSites[siteIndex])
        {
            ++attempts;
            continue;
        }

        const VoronoiSite& site = sites[siteIndex];

        PointOfInterest::Type poiType = getRandomPOIType(villagesLeft, farmsLeft, rng);

        const POITypeConfig* config = m_poiConfig.getConfig(poiType);
        if (!config)
        {
            std::cerr << "No config found for POI type\n";
            ++attempts;
            continue;
        }

        float halfWidth = config->size.x / 2.f;
        float halfHeight = config->size.y / 2.f;

        float edgeMargin = std::max(config->size.x, config->size.y) / 2.f + 200.f;

        if (site.position.x - halfWidth < edgeMargin ||
            site.position.x + halfWidth > worldSize.x - edgeMargin ||
            site.position.y - halfHeight < edgeMargin ||
            site.position.y + halfHeight > worldSize.y - edgeMargin)
        {
            std::cout << "Skipped site " << siteIndex << " - too close to map edge\n";
            usedSites[siteIndex] = true;
            ++attempts;
            continue;
        }

        int instanceNumber = (poiType == PointOfInterest::Type::Village)
            ? (settings.numVillages - villagesLeft + 1)
            : (settings.numFarms - farmsLeft + 1);

        auto poi = createPOI(poiType, site.position, instanceNumber);

        if (poi)
        {
            std::cout << "Spawned " << poi->getName() << " at Voronoi site " << siteIndex
                << " (pos: " << site.position.x << ", " << site.position.y << ")\n";

            map->addPOI(std::move(poi));
            usedSites[siteIndex] = true;
            ++poisSpawned;
        }

        ++attempts;
    }

    if (poisSpawned < totalPOIs)
    {
        std::cerr << "Warning: Only spawned " << poisSpawned << " out of "
            << totalPOIs << " requested POIs\n";
    }

    std::cout << "POI spawning complete: " << poisSpawned << " POIs placed\n";
}


std::unique_ptr<PointOfInterest> MapGenerator::createPOI(
    PointOfInterest::Type type,
    const sf::Vector2f& position,
    int instanceNumber)
{
    const POITypeConfig* config = m_poiConfig.getConfig(type);
    if (!config)
    {
        std::cerr << "Cannot create POI: No config for type\n";
        return nullptr;
    }

    std::string name = config->name + " " + std::to_string(instanceNumber);

    auto poi = std::make_unique<PointOfInterest>(
        name,
        type,
        position,
        config->size
    );

    if (!config->spritePath.empty())
    {
        if (!poi->loadSprite(config->spritePath))
        {
            std::cerr << "Warning: Failed to load sprite for " << name << "\n";
        }
    }

    if (!config->templatePath.empty())
    {
        std::string templateName = config->name;
        std::transform(templateName.begin(), templateName.end(),
            templateName.begin(), ::tolower);

        if (m_poiTemplates.hasTemplate(templateName))
        {
            m_poiTemplates.applyTemplateCollision(poi.get(), templateName);
        }
        else
        {
            std::cerr << "Warning: No template found for " << templateName << "\n";
        }
    }

    return poi;
}

PointOfInterest::Type MapGenerator::getRandomPOIType(int& villagesLeft, int& farmsLeft, std::mt19937& rng)
{
    // Count remaining POI types
    std::vector<PointOfInterest::Type> availableTypes;

    if (villagesLeft > 0)
        availableTypes.push_back(PointOfInterest::Type::Village);
    if (farmsLeft > 0)
        availableTypes.push_back(PointOfInterest::Type::Farm);

    if (availableTypes.empty())
        return PointOfInterest::Type::Landmark; // Fallback

    // Pick random type from available
    std::uniform_int_distribution<size_t> typeDist(0, availableTypes.size() - 1);
    PointOfInterest::Type selectedType = availableTypes[typeDist(rng)];

    // Decrement counter
    switch (selectedType)
    {
    case PointOfInterest::Type::Village:
        --villagesLeft;
        break;
    case PointOfInterest::Type::Farm:
        --farmsLeft;
        break;
    default:
        break;
    }

    return selectedType;
}
