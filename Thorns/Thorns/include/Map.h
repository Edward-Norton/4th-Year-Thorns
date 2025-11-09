#ifndef MAP_HPP
#define MAP_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include "MapTile.h"
#include "PointOfInterest.h"
#include "IRenderable.h"
#include "SpriteComponent.h"

/// <summary>
/// 2D grid-based map with tile data and POI management
/// Stores procedurally generated terrain and handles rendering
/// </summary>
class Map : public IRenderable
{
public:
    Map();
    ~Map() = default;

    // ========== Object Management ==========

    // Initialize map with dimensions and tile size
    void initialize(int width, int height, float tileSize);

    // Reset map data
    void reset();

    // ========== Tile Access ==========
    // Get tile at grid coordinates (bounds checked)
    MapTile* getTile(int x, int y);
    const MapTile* getTile(int x, int y) const;
    // Get tile at world position
    MapTile* getTileAtWorldPos(const sf::Vector2f& worldPos);
    const MapTile* getTileAtWorldPos(const sf::Vector2f& worldPos) const;

    // Convert world position to tile coordinates
    sf::Vector2i worldToTile(const sf::Vector2f& worldPos) const;
    // Convert tile coordinates to world position (center of tile)
    sf::Vector2f tileToWorld(int x, int y) const;

    // ========== Bounds ==========
    bool isValidTile(int x, int y) const;
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    float getTileSize() const { return m_tileSize; }
    /// Get total world size in pixels
    sf::Vector2f getWorldSize() const;

    // ========== Points of Interest ==========
    void addPOI(std::unique_ptr<PointOfInterest> poi);
    const std::vector<std::unique_ptr<PointOfInterest>>& getPOIs() const { return m_pois; }

    // Check if a world position is inside any POI
    bool isInsidePOI(const sf::Vector2f& worldPos) const;

    // Mark tiles as POI terrain based on POI bounds
    void markPOITiles();

    // ========== Rendering ==========

    void render(sf::RenderTarget& target) const override;
    void renderDebug(sf::RenderTarget& target) const;
    void renderVoronoiBoundaries(sf::RenderTarget& target) const;

    void renderVisible(sf::RenderTarget& target, const sf::View& view) const;

    void setDebugMode(bool enabled) { m_debugMode = enabled; }
    bool isDebugMode() const { return m_debugMode; }

    // ========== Sprite Management ==========
    bool loadTerrainAtlas(const std::string& atlasPath);

private:
    // ========== Tile Sprite Management ==========
    
    // Get texture rect for a terrain type from atlas
    sf::IntRect getTerrainTextureRect(MapTile::TerrainType type) const;

    // ========== Sprite Rendering ==========

    // Shared texture atlas for all terrain tiles
    mutable std::unique_ptr<SpriteComponent> m_sharedSprite;
    bool m_atlasLoaded;

    // ========== Debug Rendering (fallback) ==========

    mutable sf::VertexArray m_vertexArray;
    mutable bool m_needsRebuild;

    // ========== Data ==========
    int m_width;                // Map width in tiles
    int m_height;               // Map height in tiles
    float m_tileSize;           // Size of each tile in pixels

    std::vector<MapTile> m_tiles;  // 1D: index = y * width + x
    std::vector<std::unique_ptr<PointOfInterest>> m_pois;

    bool m_debugMode;           // Show tile colors instead of sprites
};

#endif