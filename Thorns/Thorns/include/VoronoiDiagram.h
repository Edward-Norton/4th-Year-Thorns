#ifndef VORONOI_DIAGRAM_HPP
#define VORONOI_DIAGRAM_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include <random>

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
    void generate(Map* map, int numSites, const sf::Vector2f& hideoutPos, float minSiteDistance, unsigned int seed = 0); // Random seed fallback
    
    // ========== Queries ==========
    // Find closest site to a world position
    int getClosestSiteId(const sf::Vector2f& worldPos) const;

    // Get all sites
    const std::vector<VoronoiSite>& getSites() const { return m_sites; }

    // Mark site as a POI
    void markSiteWithPOI(int siteID);
    
    // ========== Debug ==========
    void renderDebug(sf::RenderTarget& target) const;

private:
    // Generate random positions for Voronoi sites
    // Ensures sites don't spawn inside POIs or too close to them / hideout needed due to static
    void generateSites(Map* map, unsigned char numSites, const sf::Vector2f& hideoutPos, float minSiteDistance, std::mt19937& rng);
    
    // Assign each map tile to nearest Voronoi site
    void assignTilesToRegions(Map* map);
    
    // Check if position is valid for site placement
    bool isValidSitePosition(const sf::Vector2f& pos, const sf::Vector2f& hideoutPos, float minSiteDistance, float hideoutExclusion) const;
    
    // Calculate squared distance between two points
    float distanceSquared(const sf::Vector2f& a, const sf::Vector2f& b) const;

    std::vector<VoronoiSite> m_sites;
    Map* m_map;  // Non-owning pointer to map being processed
};

#endif


// Personal Notes:
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