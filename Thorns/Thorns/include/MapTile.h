#ifndef MAP_TILE_HPP
#define MAP_TILE_HPP

#include <SFML/Graphics.hpp>

class MapTile
{
public:
    enum class TerrainType
    {
        UNKNOWN,        
        Grass,          

        
        Forest,         
        DeepForest,     
        Dirt,           
        Water,          
        Stone,          

        
        POI_Collision             

    };

    MapTile();
    ~MapTile() = default;

    
    void setTerrainType(TerrainType type) { m_terrainType = type; }
    TerrainType getTerrainType() const { return m_terrainType; }

    
    void setWalkable(bool walkable) { m_walkable = walkable; }
    bool isWalkable() const { return m_walkable; }

    
    
    void setVoronoiRegion(int regionId) { m_voronoiRegion = regionId; }
    int getVoronoiRegion() const { return m_voronoiRegion; }

    
    sf::Color getDebugColor() const;

private:
    TerrainType m_terrainType;
    bool m_walkable;

    int m_voronoiRegion;    
};

#endif