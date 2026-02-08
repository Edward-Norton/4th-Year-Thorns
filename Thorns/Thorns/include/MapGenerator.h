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

        // ========== Voronoi Diagram ==========
        bool autoCalculateSites = false;                    // Auto-calculate site count from map size
        SiteDensity siteDensity = SiteDensity::Medium;      // Density for auto-calculation
        unsigned char voronoiSites = 20;       // Number of Voronoi regions
        float minSiteDistance = 400.0f; // Min distance between site in pixels
        unsigned int seed = 0;       // Random seed (0 = random)

        // ========== POI Generation ==========
        unsigned char numVillages = 1;
        unsigned char numFarms = 1;

        // ========== Object Placement (Perlin Noise) ==========
        bool enableObjectPlacement = true;      // Enable/disable Phase 2
        double objectFrequency = 0.08;          // Perlin noise frequency
        int objectOctaves = 2;                  // Number of noise layers
        double objectThreshold = 0.65;          // Placement threshold

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

private:
    // ========== Generation Phases ==========
    void phase1_Voronoi(Map* map, const GenerationSettings& settings);
    void phase2_PerlinObjects(Map* map, const GenerationSettings& settings);

    // ========== Generation Phases ==========
    unsigned char calSiteOptimalCount(const GenerationSettings& settings) const;
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