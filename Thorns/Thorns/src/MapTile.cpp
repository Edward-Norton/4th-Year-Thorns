#include "MapTile.h"

MapTile::MapTile()
    : m_terrainType(TerrainType::UNKNOWN)
    , m_walkable(false)
    , m_voronoiRegion(-1)
{
}

sf::Color MapTile::getDebugColor() const
{
    switch (m_terrainType)
    {
    case TerrainType::UNKNOWN:
        return sf::Color::Black;
    case TerrainType::Grass:
        return sf::Color(100, 200, 100); 
    case TerrainType::Forest:
        return sf::Color(50, 150, 50);   
    case TerrainType::DeepForest:
        return sf::Color(20, 100, 20);   
    case TerrainType::Dirt:
        return sf::Color(139, 90, 43);   
    case TerrainType::Water:
        return sf::Color(50, 100, 200);  
    case TerrainType::Stone:
        return sf::Color(128, 128, 128); 
    case TerrainType::POI_Collision:
        return sf::Color(200, 50, 50);   
    default:
        return sf::Color::Magenta;        
    }
}