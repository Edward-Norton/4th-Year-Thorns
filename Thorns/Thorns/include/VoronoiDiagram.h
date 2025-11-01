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
    int regionId;               // Unique ID for this region
    sf::Color debugColor;       // Color for visualization
    
    VoronoiSite(const sf::Vector2f& pos, int id)
        : position(pos)
        , regionId(id)
        , debugColor(sf::Color(
            rand() % 156 + 100,  // R: 100-255
            rand() % 156 + 100,  // G: 100-255
            rand() % 156 + 100   // B: 100-255
        ))
    {}
};

/// <summary>
/// Generates Voronoi diagram for map regions
/// Avoids placing sites inside or near Points of Interest
/// </summary>
class VoronoiDiagram
{
public:
    VoronoiDiagram();
    ~VoronoiDiagram() = default;

    // ========== Generation ==========
    // Generate Voronoi sites and assign map tiles to regions
    // Map to be passed, number of sites and the seed used for generation for next map generation
    void generate(Map* map, int numSites, unsigned int seed = 0);
    
    // ========== Queries ==========
    // Find closest site to a world position
    int getClosestSiteId(const sf::Vector2f& worldPos) const;
    
    // ========== Debug ==========
    void renderDebug(sf::RenderTarget& target) const;

private:
    // Generate random positions for Voronoi sites
    // Ensures sites don't spawn inside POIs or too close to them
    void generateSites(Map* map, int numSites, std::mt19937& rng);
    
    // Assign each map tile to nearest Voronoi site
    void assignTilesToRegions(Map* map);
    
    // Check if position is valid for site placement (not in/near POI)
    bool isValidSitePosition(const sf::Vector2f& pos, const std::vector<std::unique_ptr<PointOfInterest>>& pois) const;
    
    // Calculate squared distance between two points
    float distanceSquared(const sf::Vector2f& a, const sf::Vector2f& b) const;

    std::vector<VoronoiSite> m_sites;
    Map* m_map;  // Non-owning pointer to map being processed
};

#endif