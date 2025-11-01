#include "MapGenerator.h"
#include "PointOfInterest.h"
#include <iostream>

MapGenerator::MapGenerator()
    : m_voronoi(std::make_unique<VoronoiDiagram>())
{
}

std::unique_ptr<Map> MapGenerator::generate(const GenerationSettings& settings)
{
    std::cout << "\n=== Starting Map Generation ===\n";

    // Create map
    auto map = std::make_unique<Map>();
    map->initialize(settings.mapWidth, settings.mapHeight, settings.tileSize);

    // Setup POIs first (they block generation)
    setupStaticPOIs(map.get());
    map->markPOITiles();

    // Run generation phases
    std::cout << "\n--- Phase 1: Voronoi Diagram ---\n";
    phase1_Voronoi(map.get(), settings);

    std::cout << "\n--- Phase 2: Perlin Noise ---\n";

    std::cout << "\n--- Phase 3: Cellular Automata ---\n";

    std::cout << "\n--- Phase 4: Connectivity Check ---\n";

    std::cout << "\n=== Map Generation Complete ===\n\n";

    return map;
}

std::unique_ptr<Map> MapGenerator::generatePhase1Only(const GenerationSettings& settings)
{
    std::cout << "\n=== Generating Phase 1 Only (Voronoi) ===\n";

    auto map = std::make_unique<Map>();
    map->initialize(settings.mapWidth, settings.mapHeight, settings.tileSize);

    setupStaticPOIs(map.get());
    map->markPOITiles();

    phase1_Voronoi(map.get(), settings);

    std::cout << "=== Phase 1 Complete ===\n\n";

    return map;
}

void MapGenerator::phase1_Voronoi(Map* map, const GenerationSettings& settings)
{
    m_voronoi->generate(map, settings.voronoiSites, settings.seed);

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

void MapGenerator::setupStaticPOIs(Map* map)
{
    sf::Vector2f worldSize = map->getWorldSize();
    sf::Vector2f center(worldSize.x / 2.f, worldSize.y / 2.f);

    // Player Hideout at center
    auto hideout = std::make_unique<PointOfInterest>(
        "Player Hideout",
        PointOfInterest::Type::PlayerHideout,
        center,
        sf::Vector2f(200.f, 200.f)  // 200x200 pixel area
    );
    map->addPOI(std::move(hideout));


    ////////// REMOVE WAS USED FOR TESTING

    //Add a village in upper-right quadrant
    auto village = std::make_unique<PointOfInterest>(
        "Village",
        PointOfInterest::Type::Village,
        sf::Vector2f(worldSize.x * 0.75f, worldSize.y * 0.25f),
        sf::Vector2f(300.f, 250.f)
    );
    map->addPOI(std::move(village));

    //Add a landmark in lower-left quadrant
    auto landmark = std::make_unique<PointOfInterest>(
        "Farm",
        PointOfInterest::Type::Farm,
        sf::Vector2f(worldSize.x * 0.25f, worldSize.y * 0.75f),
        sf::Vector2f(150.f, 150.f)
    );
    map->addPOI(std::move(landmark));
}