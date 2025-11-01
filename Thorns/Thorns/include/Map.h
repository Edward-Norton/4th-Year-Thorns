#ifndef MAP_HPP
#define MAP_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include "MapTile.h"
#include "PointOfInterest.h"
#include "IRenderable.h"

/// <summary>
/// 2D grid-based map with tile data and POI management
/// Stores procedurally generated terrain and handles rendering
/// </summary>
class Map : public IRenderable
{
public:
    Map();
    ~Map() = default;

    // ========== Initialization ==========

    /// Initialize map with dimensions and tile size
    /// @param width Map width in tiles
    /// @param height Map height in tiles
    /// @param tileSize Size of each tile in pixels (e.g., 64)
    void initialize(int width, int height, float tileSize);

    // ========== Tile Access ==========

    /// Get tile at grid coordinates (bounds checked)
    MapTile* getTile(int x, int y);
    const MapTile* getTile(int x, int y) const;

    /// Get tile at world position
    MapTile* getTileAtWorldPos(const sf::Vector2f& worldPos);
    const MapTile* getTileAtWorldPos(const sf::Vector2f& worldPos) const;

    /// Convert world position to tile coordinates
    sf::Vector2i worldToTile(const sf::Vector2f& worldPos) const;

    /// Convert tile coordinates to world position (center of tile)
    sf::Vector2f tileToWorld(int x, int y) const;

    // ========== Bounds ==========

    bool isValidTile(int x, int y) const;
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    float getTileSize() const { return m_tileSize; }

    /// Get total world size in pixels
    sf::Vector2f getWorldSize() const;
    sf::FloatRect getWorldBounds() const;

    // ========== Points of Interest ==========

    void addPOI(std::unique_ptr<PointOfInterest> poi);
    const std::vector<std::unique_ptr<PointOfInterest>>& getPOIs() const { return m_pois; }

    /// Check if a world position is inside any POI
    bool isInsidePOI(const sf::Vector2f& worldPos) const;

    /// Mark tiles as POI terrain based on POI bounds
    void markPOITiles();

    // ========== Rendering ==========

    void render(sf::RenderTarget& target) const override;
    void renderDebug(sf::RenderTarget& target) const;
    void Map::renderVoronoiBoundaries(sf::RenderTarget& target) const;

    void setDebugMode(bool enabled) { m_debugMode = enabled; }
    bool isDebugMode() const { return m_debugMode; }

private:
    int m_width;                // Map width in tiles
    int m_height;               // Map height in tiles
    float m_tileSize;           // Size of each tile in pixels

    std::vector<std::vector<MapTile>> m_tiles;  // 2D grid [y][x]
    std::vector<std::unique_ptr<PointOfInterest>> m_pois;

    bool m_debugMode;           // Show tile colors instead of sprites

    // Rendering optimization (could render only visible tiles)
    mutable sf::VertexArray m_vertexArray;
    mutable bool m_needsRebuild;
};

#endif