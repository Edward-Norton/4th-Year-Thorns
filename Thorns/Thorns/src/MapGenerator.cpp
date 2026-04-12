#include "MapGenerator.h"
#include "PointOfInterest.h"
#include <iostream>
#include <chrono>
#include "AssetPaths.h"
#include <numeric>

MapGenerator::MapGenerator()
    : m_voronoi(std::make_unique<VoronoiDiagram>())
    , m_objectPlacer(std::make_unique<ObjectPlacer>())
{
    m_poiTemplates.loadTemplate("hideout", Assets::Maps::HIDEOUT_TEMPLATE);
    m_poiTemplates.loadTemplate("farm", Assets::Maps::FARM_TEMPLATE);
}

std::unique_ptr<Map> MapGenerator::generate(const GenerationSettings& settings)
{
    
    std::cout << "\n=== Starting Map Generation ===\n";

    auto startTotal = std::chrono::high_resolution_clock::now();

    
    auto map = std::make_unique<Map>();
    map->initialize(settings.mapWidth, settings.mapHeight, settings.tileSize);

    
    if (!map->loadTerrainAtlas(Assets::Textures::TERRAIN_ATLAS)) {
        std::cerr << "Failed to load the Map terrain atlas, defaulted to debug rendering\n";
        map->setDebugMode(true);
    }

    
    
    setupHideoutPOI(map.get());
    map->markPOITiles();
    

    
    
    
    std::cout << "\n--- Phase 1: Voronoi Diagram ---\n";
    phase1_Voronoi(map.get(), settings);

    
    std::cout << "\n--- Spawning POIs at Voronoi sites ---\n";
    spawnPOIsAtSites(map.get(), settings);
    map->markPOITiles();

    
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

    
    map->reset();

    
    setupHideoutPOI(map);
    map->markPOITiles();

    
    
    std::cout << "\n--- Phase 1: Voronoi Diagram ---\n";
    phase1_Voronoi(map, settings);

    
    std::cout << "\n--- Spawning POIs at Voronoi sites ---\n";
    spawnPOIsAtSites(map, settings);
    map->markPOITiles();

    
    std::cout << "\n--- Phase 2: Perlin Noise ---\n";
    if (settings.enableObjectPlacement)
    {
        phase2_PerlinObjects(map, settings);
    }

    std::cout << "\n--- Phase 3: Cellular Automata ---\n";

    std::cout << "\n--- Phase 4: Connectivity Check ---\n";

    std::cout << "\n=== Map Regeneration Complete ===\n\n";
}

std::vector<sf::Vector2f> MapGenerator::getEnemySpawnPoints(int countPerPOI, float spawnRadius) const
{
    std::vector<sf::Vector2f> points;

    const auto& sites = m_voronoi->getSites();
    if (sites.empty()) return points;

    
    const sf::Vector2f offsets[] = {
        { spawnRadius,  0.f },
        { -spawnRadius, 0.f },
        { 0.f,  spawnRadius },
        { 0.f, -spawnRadius }
    };

    int offsetCount = std::min(countPerPOI, 4);

    
    for (size_t i = 1; i < sites.size(); ++i)
    {
        for (int j = 0; j < offsetCount; ++j)
            points.push_back(sites[i].position + offsets[j]);
    }

    return points;
}

std::vector<sf::Vector2f> MapGenerator::getItemSpawnPoints(float spawnRadius) const
{
    std::vector<sf::Vector2f> points;

    const auto& sites = m_voronoi->getSites();
    if (sites.empty()) return points;

    for (size_t i = 1; i < sites.size(); ++i)
    {
        points.push_back(sites[i].position + sf::Vector2f(spawnRadius * 0.7f, spawnRadius * 0.7f));
    }

    return points;
}

void MapGenerator::phase1_Voronoi(Map* map, const GenerationSettings& settings)
{
    
    unsigned char siteCount = 0;
    float effectiveMinDist = 0.f;

    
    std::visit([&](auto&& mode)
    {
            
            using T = std::decay_t<decltype(mode)>;

            if constexpr (std::is_same_v<T, GenerationSettings::ManualSites>)
            {
                
                siteCount = mode.count;
                effectiveMinDist = settings.deriveMinSiteDistance(siteCount);
                std::cout << "Site mode: Manual (" << static_cast<int>(siteCount) << " sites)\n";
            }
            else if constexpr (std::is_same_v<T, GenerationSettings::AutoDensity>)
            {
                
                siteCount = calSiteOptimalCount(mode.density, settings);
                effectiveMinDist = settings.deriveMinSiteDistance(siteCount);
                std::cout << "Site mode: Auto density (" << static_cast<int>(siteCount) << " sites)\n";
            }
    }, settings.siteMode);

    
    if (settings.minSiteDistance > 0.f)
    {
        effectiveMinDist = settings.minSiteDistance;
        std::cout << "minSiteDistance: " << effectiveMinDist << "px (manual override)\n";
    }
    else
    {
        std::cout << "minSiteDistance: " << effectiveMinDist << "px (auto derived from site count)\n";
    }

    std::cout << "Generating Voronoi diagram with " << static_cast<int>(siteCount) << " sites\n";

    
    std::mt19937 rng(settings.seed == 0 ? std::random_device{}() : settings.seed);
    sf::Vector2f worldSize = map->getWorldSize();

    m_voronoi->generateSitesPoisson(map, siteCount, m_hideoutPosition,
        effectiveMinDist, rng);

    
    
    const auto& sites = m_voronoi->getSites();
    std::cout << "Building spatial grid for " << sites.size() << " sites...\n";

    m_voronoi->buildSpatialGrid(worldSize.x, worldSize.y, effectiveMinDist);

    
    for (size_t i = 0; i < sites.size(); ++i)
    {
        m_voronoi->addSiteToGrid(static_cast<int>(i), sites[i].position);
    }

    
    
    int width = map->getWidth();
    int height = map->getHeight();
    int tilesProcessed = 0;

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            MapTile* tile = map->getTile(x, y);
            if (!tile)
                continue;

            
            if (tile->getTerrainType() == MapTile::TerrainType::POI_Collision)
                continue;

            sf::Vector2f tilePos = map->tileToWorld(x, y);

            
            std::vector<int> nearbySites = m_voronoi->getNearbySites(tilePos);

            int closestRegion = -1;
            float closestDistSq = std::numeric_limits<float>::max();

            
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

            tile->setVoronoiRegion(closestRegion);
            tile->setTerrainType(MapTile::TerrainType::Grass);
            tile->setWalkable(true);

            ++tilesProcessed;
        }
    }

    std::cout << "Voronoi assignment complete: " << tilesProcessed << " tiles processed\n";

    
    std::unordered_map<int, int> regionCounts;
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            const MapTile* tile = map->getTile(x, y);
            if (tile && tile->getVoronoiRegion() != -1)
                regionCounts[tile->getVoronoiRegion()]++;
        }
    }

    std::cout << "Tiles per region:\n";
    for (const auto& [regionId, count] : regionCounts)
    {
        std::cout << "  Region " << regionId << ": " << count << " tiles\n";
    }
}

void MapGenerator::phase2_PerlinObjects(Map* map, const GenerationSettings& settings)
{
    std::cout << "\n--- Phase 2: Perlin Noise Object Placement ---\n";

    
    if (!m_perlinAssetsInit)
    {
        if (!m_objectPlacer->initialize(
            Assets::Textures::FOREST_ATLAS,
            Assets::Data::FOREST_ATLAS_DEFINITIONS,
            Assets::Maps::WORLD_OBJECTS_TEMPLATE))
        {
            std::cerr << "Failed to initialize ObjectPlacer!\n";
            return;
        }
        m_perlinAssetsInit = true;
    }

    
    ObjectPlacer::PlacementSettings placementSettings;
    placementSettings.frequency = settings.objectFrequency;
    placementSettings.octaves = settings.objectOctaves;
    placementSettings.persistence = 0.5;
    placementSettings.placementThreshold = settings.objectThreshold;
    placementSettings.objectType = WorldObject::Type::SmallRoot;  
    placementSettings.respectPOIs = true;  
    placementSettings.grassOnly = true;    

    
    m_objectPlacer->generateObjects(map, placementSettings, settings.seed);

    std::cout << "Phase 2 complete: " << m_objectPlacer->getObjectCount() << " objects placed\n";
}

unsigned char MapGenerator::calSiteOptimalCount(SiteDensity density, const GenerationSettings& settings) const
{
    
    float worldWidth = settings.mapWidth * settings.tileSize;
    float worldHeight = settings.mapHeight * settings.tileSize;
    float totalArea = worldWidth * worldHeight;

    
    float areaPerSite = getAreaPerSite(density);

    
    float siteCountFloat = totalArea / areaPerSite;
    unsigned char siteCount = static_cast<unsigned char>(std::round(siteCountFloat));
    
    siteCount = std::max<unsigned char>(3, std::min<unsigned char>(255, siteCount));

    std::cout << "Auto-calculated site count:\n"
        << "  Map area: " << totalArea << " px\n"
        << "  Area per site: " << areaPerSite << " px per site\n"
        << "  Calculated sites: " << static_cast<int>(siteCount) << "\n";

    return siteCount;
}

float MapGenerator::getAreaPerSite(SiteDensity density) const
{
    
    
    switch (density)
    {
    case SiteDensity::Sparce:
        return 2000000.0f;  
    case SiteDensity::Medium:
        return 1000000.0f;  
    case SiteDensity::Dense:
        return 500000.0f;   
    default:
        return 1000000.0f;
    }
}

void MapGenerator::setupHideoutPOI(Map* map)
{
    sf::Vector2f worldSize = map->getWorldSize();
    m_hideoutPosition = sf::Vector2f(worldSize.x / 2.f, worldSize.y / 2.f);

    
    auto hideout = std::make_unique<PointOfInterest>(
        "Player Hideout",
        PointOfInterest::Type::PlayerHideout,
        m_hideoutPosition,
        sf::Vector2f(481.0f, 419.0f)
    );

    
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

    
    
    std::vector<int> sitesByDistance(sites.size()); 
    std::iota(sitesByDistance.begin(), sitesByDistance.end(), 0); 

    
    std::sort(sitesByDistance.begin(), sitesByDistance.end(), [&](int a, int b)
    {
            auto distSq = [&](const sf::Vector2f& pos)
            {
                    float dx = pos.x - m_hideoutPosition.x;
                    float dy = pos.y - m_hideoutPosition.y;
                    return dx * dx + dy * dy;
            };
            
            return distSq(sites[a].position) > distSq(sites[b].position);
    });

    std::mt19937 rng(settings.seed == 0 ? std::random_device{}() : settings.seed);
    std::uniform_int_distribution<int> siteDist(0, static_cast<int>(sites.size()) - 1);

    int villagesLeft = settings.numVillages;
    int farmsLeft = settings.numFarms;
    std::vector<bool> usedSites(sites.size(), false);

    sf::Vector2f worldSize = map->getWorldSize();

    int poisSpawned = 0;
    int attempts = 0;
    const int maxAttempts = totalPOIs * 20;

    
    for (int siteIndex : sitesByDistance)
    {
        if (poisSpawned >= totalPOIs)
            break;

        const VoronoiSite& site = sites[siteIndex];

        PointOfInterest::Type poiType = getRandomPOIType(villagesLeft, farmsLeft, rng);

        const POITypeConfig* config = m_poiConfig.getConfig(poiType);
        if (!config)
        {
            std::cerr << "No config found for POI type\n";
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

            
            if (poiType == PointOfInterest::Type::Village) ++villagesLeft;
            else if (poiType == PointOfInterest::Type::Farm) ++farmsLeft;
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
            ++poisSpawned;
        }
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
    
    std::vector<PointOfInterest::Type> availableTypes;

    if (villagesLeft > 0)
        availableTypes.push_back(PointOfInterest::Type::Village);
    if (farmsLeft > 0)
        availableTypes.push_back(PointOfInterest::Type::Farm);

    if (availableTypes.empty())
        return PointOfInterest::Type::Landmark; 

    
    std::uniform_int_distribution<size_t> typeDist(0, availableTypes.size() - 1);
    PointOfInterest::Type selectedType = availableTypes[typeDist(rng)];

    
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
