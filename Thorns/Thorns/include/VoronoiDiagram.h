#ifndef VORONOI_DIAGRAM_HPP
#define VORONOI_DIAGRAM_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include <random>
#include <unordered_map>

class Map;
class PointOfInterest;

/// <summary>
/// Voronoi site (seed point) that generates a region
/// </summary>
struct VoronoiSite
{
    sf::Vector2f position;      // World position of site
    sf::Vector2i tileCoords;    // Tile linked to the site
    int regionId;               // Unique ID for this region
    sf::Color debugColor;       // Color for visualization
    bool hasPOI;                // If the site has a POI linked to it
    
    VoronoiSite(const sf::Vector2f& pos,const sf::Vector2i& coords, int id)
        : position(pos)
        , tileCoords(coords)
        , regionId(id)
        , hasPOI(false)
        , debugColor(sf::Color(
            rand() % 156 + 100,  // R: 100-255
            rand() % 156 + 100,  // G: 100-255
            rand() % 156 + 100   // B: 100-255
        ))
    {}
};

/// <summary>
/// Spatial grid for fast nearest-neighbor queries
/// Divides world space into cells for O(1) site lookups
/// </summary>
class SpatialGrid
{
public:
    SpatialGrid();

    void initialize(float worldWidth, float worldHeight, float cellSize);
    void clear();

    // Add site to grid
    void addSite(int siteIndex, const sf::Vector2f& position);

    // Get sites near a position (returns indices into VoronoiDiagram::m_sites)
    std::vector<int> getNearbySites(const sf::Vector2f& position) const;

private:
    float m_cellSize;
    int m_gridWidth;
    int m_gridHeight;
    float m_worldWidth;
    float m_worldHeight;

    // Hash grid: (gridX, gridY) -> list of site indices
    std::unordered_map<int, std::vector<int>> m_cells;

    // Convert world position to grid coordinates
    sf::Vector2i worldToGrid(const sf::Vector2f& position) const;

    // Convert grid coordinates to hash key
    int gridToHash(int gridX, int gridY) const;
};

/// <summary>
/// Generates Voronoi diagram for map regions
/// Tries to create exclusion zones around the sites
/// </summary>
class VoronoiDiagram
{
public:
    VoronoiDiagram();
    ~VoronoiDiagram() = default;

    // ========== Generation ==========
    // Generate Voronoi sites and assign map tiles to regions
    // Map to be passed, number of sites and the seed used for generation for next map generation
    // *Note: Respects Hideout position in the generation for min spacing 
    // ========== Poisson Disk Sampling ==========
    // Generate evenly-spaced sites using Bridson's algorithm
    void generateSitesPoisson(Map* map, unsigned char numSites,
        const sf::Vector2f& hideoutPos,
        float minSiteDistance, std::mt19937& rng);

    // ========== Rejection Sampling ==========
    // Old method; kept for comparison and documentation
    void VoronoiDiagram::generateSitesRejection(Map* map, unsigned char numSites,
        const sf::Vector2f& hideoutPos,
        float minSiteDistance, std::mt19937& rng);


    // Build spatial grid
    void buildSpatialGrid(float worldWidth, float worldHeight, float cellSize)
    {
        m_spatialGrid.initialize(worldWidth, worldHeight, cellSize);
    }

    // Add site to spatial grid
    void addSiteToGrid(int siteIndex, const sf::Vector2f& position)
    {
        m_spatialGrid.addSite(siteIndex, position);
    }

    // Get nearby sites for a position (exposed for single-pass)
    std::vector<int> getNearbySites(const sf::Vector2f& position) const
    {
        return m_spatialGrid.getNearbySites(position);
    }
    
    // ========== Queries ==========
    // Find closest site to a world position
    int getClosestSiteId(const sf::Vector2f& worldPos) const;

    // Get all sites
    const std::vector<VoronoiSite>& getSites() const { return m_sites; }

    // Mark site as a POI
    void markSiteWithPOI(int siteID);
    
    // ========== Debug ==========
    void renderDebug(sf::RenderTarget& target) const;

    // ========== Wipe Old Data ==========
    void clear()
    {
        m_sites.clear();
        m_spatialGrid.clear();
        m_poissonGrid.clear();
        m_activeList.clear();
    }

private:
    // ========== Poisson Disk Sampling ==========
    // Helper: Check if position is valid for new site
    bool isValidSitePositionPoisson(const sf::Vector2f& pos,
        const sf::Vector2f& hideoutPos,
        float minDist, float hideoutExclusion) const;

    // ========== Rejection Sampling ==========
    // Check if position is valid for site placement
    bool isValidSitePosition(const sf::Vector2f& pos, const sf::Vector2f& hideoutPos, 
        float minSiteDistance, float hideoutExclusion) const;
    
    // ========== Spatial Partitioning ==========
    // Assign tiles to regions using spatial grid for fast lookups
    void assignTilesToRegionsSP(Map* map);
    
    
    // ========== Utilities ==========
    // Calculate squared distance between two points
    float distanceSquared(const sf::Vector2f& a, const sf::Vector2f& b) const;

    // Locals
    std::vector<VoronoiSite> m_sites;
    SpatialGrid m_spatialGrid;
    Map* m_map;  // Non-owning pointer to map being processed

    // Poisson dusk sampling temp stoages
    mutable std::vector<sf::Vector2f> m_poissonGrid;
    mutable std::vector<sf::Vector2f> m_activeList;
};

#endif


// Personal Notes Voronoi Diagram:
/*
    Voronoi Diagrams are cell wall like structured, seperating regions based on distance into seed points.]
    Each point in space belongs its closets seed point.

    I.e with 3 sites:
    - Every tile closer to A than B or C is part of A's region

    Algorithim overview:
    // Source Urls: https://leatherbee.org/index.php/2018/10/06/terrain-generation-3-voronoi-diagrams/
                 // https://www.geeksforgeeks.org/dsa/voronoi-diagram/
    
    Phase 1: Generate the sites
    - Pick random positions for each N site on map
    - Enforce a min distance to prevent cramping sites on top of each other
    - Avoid placing near the hideout 
    - Each site gets a unique ID

    Phase 2: Region Assignment:
    - For each tile in the map
            - Calculate distance
            - Assign tile to nearest region
    
    Rejection Sampling:
    - Had to add these for optimization due to long compiler times on own machine at times
        - Invalid postions rejected
        - Max attempts due to infinte loop possibilities
*/

// Personal Notes Poisson Sampling and Spatial Partition:
/*
    // Video Url for Poisson example: https://youtu.be/jofUe3Kjag4?si=HnIRv3t2Yv7CT_Gq
*/