#ifndef MAP_GENERATOR_HPP
#define MAP_GENERATOR_HPP

#include <memory>
#include "Map.h"
#include "VoronoiDiagram.h"

/// <summary>
/// Orchestrates procedural map generation pipeline
/// Phase 1: Voronoi diagram for regionalization
///         - Divide map into regions
///         - Terrain boundaries
///         - Ensure POIs are spaced appropriately
/// Phase 2: (TODO) Perlin noise for terrain density
/// Phase 3: (TODO) Cellular automata for refinement
/// Phase 4: (TODO) Dijkstra map for connectivity validation
/// </summary>
class MapGenerator
{
public:
    struct GenerationSettings
    {
        // ========== Map Layout ==========
        unsigned short mapWidth = 128 ;          // Map width in tiles
        unsigned short mapHeight = 128;         // Map height in tiles
        float tileSize = 64.f;       // Tile size in pixels

        // ========== Voronoi Diagram ==========
        unsigned char voronoiSites = 20;       // Number of Voronoi regions
        float minSiteDistance = 400.0f; // Min distance between site in pixels
        unsigned int seed = 0;       // Random seed (0 = random)

        // ========== POI Generation ==========
        unsigned char numVillages = 1;
        unsigned char numFarms = 1;

    };

    MapGenerator();
    ~MapGenerator() = default;

    // ========== Generation ==========
    // Generate complete map with all phases
    std::unique_ptr<Map> generate(const GenerationSettings& settings);

    // Regenerate existing map with new seed
    void regenerate(Map* map, const GenerationSettings& settings);

    // ========== Phase Access (for debugging) ==========
    VoronoiDiagram* getVoronoiDiagram() { return m_voronoi.get(); }

private:
    // ========== Generation Phases ==========
    void phase1_Voronoi(Map* map, const GenerationSettings& settings);

    // ========== POI ==========
    // Add default POIs to map (hideout)
    void setupHideoutPOI(Map* map);

    // Spawn POIs at Voronoi sites based on settings
    void spawnPOIsAtSites(Map* map, const GenerationSettings& settings);

    // Helper to pick random POI type
    PointOfInterest::Type getRandomPOIType(int& villagesLeft, int& farmsLeft,std::mt19937& rng);

    // ========== Algorithm Instances ========== (Only pointers incase pointer to phase later is needed)
    std::unique_ptr<VoronoiDiagram> m_voronoi;

    // Store hideout position for Voronoi generation
    sf::Vector2f m_hideoutPosition;
};

#endif