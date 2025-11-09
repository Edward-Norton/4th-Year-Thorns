#include "MapGenerator.h"
#include "PointOfInterest.h"
#include <iostream>
#include <chrono>
#include "AssetPaths.h"

MapGenerator::MapGenerator()
    : m_voronoi(std::make_unique<VoronoiDiagram>())
{
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

    std::cout << "\n--- Phase 3: Cellular Automata ---\n";

    std::cout << "\n--- Phase 4: Connectivity Check ---\n";

    std::cout << "\n=== Map Generation Complete ===\n\n";

    auto endTotal = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double, std::milli> elapsed = endTotal - startTotal;

    std::cout << "mapGeneration took " << elapsed.count() << " ms\n";


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

    std::cout << "Generating Voronoi sites with Poisson disk sampling...\n";
    m_voronoi->generateSitesPoisson(map, settings.voronoiSites, m_hideoutPosition,
        settings.minSiteDistance, rng);

    // Step 2: Build spatial grid for fast nearest-neighbor queries
    const auto& sites = m_voronoi->getSites();
    std::cout << "Building spatial grid for " << sites.size() << " sites...\n";

    m_voronoi->buildSpatialGrid(worldSize.x, worldSize.y, settings.minSiteDistance);

    for (size_t i = 0; i < sites.size(); ++i)
    {
        m_voronoi->addSiteToGrid(static_cast<int>(i), sites[i].position);
    }

    // Step 3: Single loop - assign regions AND set terrain together
    std::cout << "Assigning tiles to regions in single pass...\n";

    int width = map->getWidth();
    int height = map->getHeight();
    int tilesProcessed = 0;

    // ========== SINGLE LOOP: Do everything in one pass ==========
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            MapTile* tile = map->getTile(x, y);
            if (!tile)
                continue;

            // Skip POI tiles (hideout area)
            if (tile->getTerrainType() == MapTile::TerrainType::POI)
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

            // Set terrain type (all in same pass!)
            tile->setTerrainType(MapTile::TerrainType::Grass);
            tile->setWalkable(true);

            ++tilesProcessed;
        }
    }

    // After the region assignment loop, add this:
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
        sf::Vector2f(200.f, 200.f)  // 200x200 pixel area
    );

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

    // Calculate total POIs to spawn
    int totalPOIs = settings.numVillages + settings.numFarms;

    if (totalPOIs > static_cast<int>(sites.size()))
    {
        std::cerr << "Warning: Requested " << totalPOIs << " POIs but only "
            << sites.size() << " sites available!\n";
        totalPOIs = static_cast<int>(sites.size());
    }

    // Setup RNG for random site selection
    std::mt19937 rng(settings.seed == 0 ? std::random_device{}() : settings.seed + 999);
    std::uniform_int_distribution<int> siteDist(0, static_cast<int>(sites.size()) - 1);

    // Track remaining POIs to spawn
    int villagesLeft = settings.numVillages;
    int farmsLeft = settings.numFarms;

    // Track used site indices
    std::vector<bool> usedSites(sites.size(), false);

    // Spawn POIs
    int poisSpawned = 0;
    int attempts = 0;
    const int maxAttempts = totalPOIs * 10;

    while (poisSpawned < totalPOIs && attempts < maxAttempts)
    {
        // Pick random site
        int siteIndex = siteDist(rng);

        // Skip if already used
        if (usedSites[siteIndex])
        {
            ++attempts;
            continue;
        }

        const VoronoiSite& site = sites[siteIndex];

        // Determine POI type
        PointOfInterest::Type poiType = getRandomPOIType(villagesLeft, farmsLeft, rng);

        // Get name and size based on type
        std::string name;
        sf::Vector2f size;

        switch (poiType)
        {
        case PointOfInterest::Type::Village:
            name = "Village " + std::to_string(settings.numVillages - villagesLeft + 1);
            size = sf::Vector2f(300.f, 250.f);
            break;
        case PointOfInterest::Type::Farm:
            name = "Farm " + std::to_string(settings.numFarms - farmsLeft + 1);
            size = sf::Vector2f(110.f, 100.f);
            break;
        default:
            name = "Unknown POI";
            size = sf::Vector2f(150.f, 150.f);
            break;
        }

        // Create POI at site position
        auto poi = std::make_unique<PointOfInterest>(name, poiType, site.position, size);

        std::cout << "Spawned " << name << " at Voronoi site " << siteIndex
            << " (pos: " << site.position.x << ", " << site.position.y << ")\n";

        map->addPOI(std::move(poi));

        // Mark site as used and having POI
        usedSites[siteIndex] = true;
        m_voronoi->markSiteWithPOI(site.regionId);

        ++poisSpawned;
        ++attempts;
    }

    if (poisSpawned < totalPOIs)
    {
        std::cerr << "Warning: Only spawned " << poisSpawned << " out of "
            << totalPOIs << " requested POIs\n";
    }

    std::cout << "POI spawning complete: " << poisSpawned << " POIs placed\n";
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
