#ifndef VORONOI_DIAGRAM_HPP
#define VORONOI_DIAGRAM_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include <random>
#include <unordered_map>

class Map;
class PointOfInterest;

struct VoronoiSite
{
    sf::Vector2f position;      
    sf::Vector2i tileCoords;    
    int regionId;               
    sf::Color debugColor;       
    bool hasPOI;                
    
    VoronoiSite(const sf::Vector2f& pos,const sf::Vector2i& coords, int id)
        : position(pos)
        , tileCoords(coords)
        , regionId(id)
        , hasPOI(false)
        , debugColor(sf::Color(
            rand() % 156 + 100,  
            rand() % 156 + 100,  
            rand() % 156 + 100   
        ))
    {}
};

class SpatialGrid
{
public:
    SpatialGrid();

    void initialize(float worldWidth, float worldHeight, float cellSize);
    void clear();

    
    void addSite(int siteIndex, const sf::Vector2f& position);

    
    std::vector<int> getNearbySites(const sf::Vector2f& position) const;

private:
    float m_cellSize;
    int m_gridWidth;
    int m_gridHeight;
    float m_worldWidth;
    float m_worldHeight;

    
    std::unordered_map<int, std::vector<int>> m_cells;

    
    sf::Vector2i worldToGrid(const sf::Vector2f& position) const;

    
    int gridToHash(int gridX, int gridY) const;
};

class VoronoiDiagram
{
public:
    VoronoiDiagram();
    ~VoronoiDiagram() = default;

    
    
    
    
    
    
    void generateSitesPoisson(Map* map, unsigned char numSites,
        const sf::Vector2f& hideoutPos,
        float minSiteDistance, std::mt19937& rng);

    
    
    void VoronoiDiagram::generateSitesRejection(Map* map, unsigned char numSites,
        const sf::Vector2f& hideoutPos,
        float minSiteDistance, std::mt19937& rng);

    
    void buildSpatialGrid(float worldWidth, float worldHeight, float cellSize)
    {
        m_spatialGrid.initialize(worldWidth, worldHeight, cellSize);
    }

    
    void addSiteToGrid(int siteIndex, const sf::Vector2f& position)
    {
        m_spatialGrid.addSite(siteIndex, position);
    }

    
    std::vector<int> getNearbySites(const sf::Vector2f& position) const
    {
        return m_spatialGrid.getNearbySites(position);
    }
    
    
    
    int getClosestSiteId(const sf::Vector2f& worldPos) const;

    
    const std::vector<VoronoiSite>& getSites() const { return m_sites; }

    
    void markSiteWithPOI(int siteID);
    
    
    void renderDebug(sf::RenderTarget& target) const;

    
    void clear()
    {
        m_sites.clear();
        m_spatialGrid.clear();
        m_poissonGrid.clear();
        m_activeList.clear();
    }

private:
    
    
    bool isValidSitePositionPoisson(const sf::Vector2f& pos,
        const sf::Vector2f& hideoutPos,
        float minDist, float hideoutExclusion) const;

    
    
    bool isValidSitePosition(const sf::Vector2f& pos, const sf::Vector2f& hideoutPos, 
        float minSiteDistance, float hideoutExclusion) const;
    
    
    
    void assignTilesToRegionsSP(Map* map);
    
    
    
    
    float distanceSquared(const sf::Vector2f& a, const sf::Vector2f& b) const;

    
    std::vector<VoronoiSite> m_sites;
    SpatialGrid m_spatialGrid;
    Map* m_map;  

    
    mutable std::vector<sf::Vector2f> m_poissonGrid;
    mutable std::vector<sf::Vector2f> m_activeList;
};

#endif

