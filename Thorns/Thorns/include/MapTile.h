#ifndef MAP_TILE_HPP
#define MAP_TILE_HPP

#include <SFML/Graphics.hpp>

/// <summary>
/// Individual tile in the procedural map
/// Stores terrain type, walkability, region data, etc.
/// </summary>
class MapTile
{
public:
    enum class TerrainType
    {
        UNKNOWN,        // Ungenerated/invalid
        Grass,          // Open grassland
        // For later use
        Forest,         // Dense forest
        DeepForest,     // Very dense forest
        Dirt,           // Cleared/path areas
        Water,          // Lakes/rivers
        Stone,          // Rocky terrain or boulders
        // These are red for now
        POI             // Part of a Point of Interest
    };

    MapTile();
    ~MapTile() = default;

    // ========== Terrain ==========
    void setTerrainType(TerrainType type) { m_terrainType = type; }
    TerrainType getTerrainType() const { return m_terrainType; }

    // ========== Walkability ==========
    void setWalkable(bool walkable) { m_walkable = walkable; }
    bool isWalkable() const { return m_walkable; }

    // ========== Voronoi Data ==========
    // Set which Voronoi region this tile belongs to
    void setVoronoiRegion(int regionId) { m_voronoiRegion = regionId; }
    int getVoronoiRegion() const { return m_voronoiRegion; }

    // ========== Rendering ==========
    sf::Color getDebugColor() const;

private:
    TerrainType m_terrainType;
    bool m_walkable;

    int m_voronoiRegion;    // Which Voronoi cell this tile belongs to (-1 = none)
};

#endif