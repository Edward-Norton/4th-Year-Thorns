#include "MapGenerator.h"
#include "PointOfInterest.h"
#include <iostream>

MapGenerator::MapGenerator()
    : m_voronoi(std::make_unique<VoronoiDiagram>())
{
}

std::unique_ptr<Map> MapGenerator::generate(const GenerationSettings& settings)
{
    // Step 2: Generate the base map (pn step 1 in game.cpp)
    std::cout << "\n=== Starting Map Generation ===\n";

    // Step 2.1: All tiles are UNKNOWN initally and use the default params while also alocating memory for map
    auto map = std::make_unique<Map>();
    map->initialize(settings.mapWidth, settings.mapHeight, settings.tileSize);

    // Setup static POIs (hideout at the center)
    // Step 2.3: Where the hideout needs to be based on map size
    setupHideoutPOI(map.get());
    map->markPOITiles();
    

    // Run generation phases
    // ========== VORONOI DIAGRAMS ==========
    //Step 3: Seeds Voronoi
    std::cout << "\n--- Phase 1: Voronoi Diagram ---\n";
    phase1_Voronoi(map.get(), settings);

    // Spawn POIs at Voronoi sites
    std::cout << "\n--- Spawning POIs at Voronoi sites ---\n";
    spawnPOIsAtSites(map.get(), settings);
    map->markPOITiles();

    // ========== PERLIN NOISE ==========
    std::cout << "\n--- Phase 2: Perlin Noise ---\n";

    std::cout << "\n--- Phase 3: Cellular Automata ---\n";

    std::cout << "\n--- Phase 4: Connectivity Check ---\n";

    std::cout << "\n=== Map Generation Complete ===\n\n";

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


void MapGenerator::phase1_Voronoi(Map* map, const GenerationSettings& settings)
{
    // Generate Voronoi with hideout awareness
    m_voronoi->generate(map, settings.voronoiSites, m_hideoutPosition,
        settings.minSiteDistance, settings.seed);

    // Color tiles based on their Voronoi region for visualization
    for (int y = 0; y < map->getHeight(); ++y)
    {
        for (int x = 0; x < map->getWidth(); ++x)
        {
            MapTile* tile = map->getTile(x, y);
            if (!tile)
                continue;

            // Skip POI tiles
            if (tile->getTerrainType() == MapTile::TerrainType::POI)
                continue;

            // For now, just mark as grass (will be refined in later phases)
            tile->setTerrainType(MapTile::TerrainType::Grass);
            tile->setWalkable(true);
        }
    }
}

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
        std::cerr << "No Voronoi sites available for POI placement!\n";
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
