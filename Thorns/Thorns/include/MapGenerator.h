#ifndef MAP_GENERATOR_HPP
#define MAP_GENERATOR_HPP

#include <memory>
#include "Map.h"
#include "VoronoiDiagram.h"
#include "ObjectPlacer.h"
#include "POITemplate.h"
#include "POITypeConfig.h"

/// <summary>
/// Orchestrates procedural map generation pipeline
/// Phase 1: Voronoi diagram for regionalization
///         - Divide map into regions
///         - Terrain boundaries
///         - Ensure POIs are spaced appropriately
/// Phase 2: Perlin noise for object placement
///         - Trees, rocks, vegetation
///         - Natural-looking distribution
/// Phase 3: (TODO) Cellular automata for refinement
/// Phase 4: (TODO) Dijkstra map for connectivity validation
/// </summary>
class MapGenerator
{
public:
    enum class SiteDensity
    {
        Sparce,  // Less, larger regions 
        Medium, // Balanced,
        Dense,  // More, Smaller regions
    };


    struct GenerationSettings
    {
        // ========== Map Layout ==========
        unsigned short mapWidth = 128 ;          // Map width in tiles
        unsigned short mapHeight = 128;         // Map height in tiles
        float tileSize = 64.f;       // Tile size in pixels

        // ========== Voronoi Site Generation ==========
        // Two mutually exclusive modes to try and make it easier to swap if needed for testing or final version
        // ManualSites: explicit site count, spacing auto-derived from count and map area
        // AutoDensity: site count and spacing both derived from map area + density preset
        struct ManualSites { unsigned char count = 20; }; // Number of Voronoi regions
        struct AutoDensity { SiteDensity density = SiteDensity::Medium; }; // Density for auto-calculation 

        // Holds either ManualSites or AutoDensity
        // Basically just a swap for types.
        std::variant<ManualSites, AutoDensity> siteMode = ManualSites{};
 
        float minSiteDistance = 0.0f; // Min distance between site in pixels, leave at 0 of for the auto derive
        unsigned int seed = 0;       // Random seed (0 = random)


        // ========== POI Generation ==========
        unsigned char numVillages = 1;
        unsigned char numFarms = 1;

        // ========== Object Placement (Perlin Noise) ==========
        bool enableObjectPlacement = true;      // Enable/disable Phase 2
        double objectFrequency = 0.08;          // Perlin noise frequency
        int objectOctaves = 2;                  // Number of noise layers
        double objectThreshold = 0.65;          // Placement threshold

        // Derives the min distance for the current way of setting points (Poisson), 
        // make each site own an even share of the map, solve issue with sites not covering map sections
        // Get size total, divide to get each area each site roughly holds, distance for sites
        float deriveMinSiteDistance(int count) const
        {
            float worldW = static_cast<float>(mapWidth) * tileSize;
            float worldH = static_cast<float>(mapHeight) * tileSize;

            // sqrt(area / n) gives the side of a square that each site would own.
            // Multiplied by 0.85 to allow Poisson to fill the space without running out of room.
            return std::sqrt((worldW * worldH) / static_cast<float>(std::max(1, count))) * 0.85f;
        }


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
    ObjectPlacer* getObjectPlacer() { return m_objectPlacer.get(); }


    // Enemy spawning
    std::vector<sf::Vector2f> getEnemySpawnPoints(int countPerPOI = 2, float spawnRadius = 300.f) const;

private:
    // ========== Generation Phases ==========
    void phase1_Voronoi(Map* map, const GenerationSettings& settings);
    void phase2_PerlinObjects(Map* map, const GenerationSettings& settings);

    // ========== Generation Phases ==========
    unsigned char calSiteOptimalCount(SiteDensity density, const GenerationSettings& settings) const;
    // Get area per site for given density preset
    float getAreaPerSite(SiteDensity density) const;

    // ========== POI ==========
    // Add default POIs to map (hideout)
    void setupHideoutPOI(Map* map);

    // Spawn POIs at Voronoi sites based on settings
    void spawnPOIsAtSites(Map* map, const GenerationSettings& settings);

    POIConfigRegistry m_poiConfig;
    POITemplateManager m_poiTemplates;

    std::unique_ptr<PointOfInterest> createPOI(
        PointOfInterest::Type type,
        const sf::Vector2f& position,
        int instanceNumber);

    // Helper to pick random POI type
    PointOfInterest::Type getRandomPOIType(int& villagesLeft, int& farmsLeft,std::mt19937& rng);

    // ========== Algorithm Instances ========== (Only pointers incase pointer to phase later is needed)
    std::unique_ptr<VoronoiDiagram> m_voronoi;
    std::unique_ptr<ObjectPlacer> m_objectPlacer;

    // Store hideout position for Voronoi generation
    sf::Vector2f m_hideoutPosition;

    // Other locals
    bool m_perlinAssetsInit = false;

};

#endif