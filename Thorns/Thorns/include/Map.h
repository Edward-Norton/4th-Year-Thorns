#ifndef MAP_HPP
#define MAP_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include "MapTile.h"
#include "PointOfInterest.h"
#include "IRenderable.h"
#include "SpriteComponent.h"

class Map : public IRenderable
{
public:
    Map();
    ~Map() = default;

    

    
    void initialize(int width, int height, float tileSize);

    
    void reset();

    
    
    MapTile* getTile(int x, int y);
    const MapTile* getTile(int x, int y) const;
    
    MapTile* getTileAtWorldPos(const sf::Vector2f& worldPos);
    const MapTile* getTileAtWorldPos(const sf::Vector2f& worldPos) const;

    
    sf::Vector2i worldToTile(const sf::Vector2f& worldPos) const;
    
    sf::Vector2f tileToWorld(int x, int y) const;

    
    bool isValidTile(int x, int y) const;
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    float getTileSize() const { return m_tileSize; }
    
    sf::Vector2f getWorldSize() const;

    
    void addPOI(std::unique_ptr<PointOfInterest> poi);
    const std::vector<std::unique_ptr<PointOfInterest>>& getPOIs() const { return m_pois; }

    
    bool isInsidePOI(const sf::Vector2f& worldPos) const;

    
    void markPOITiles();

    

    void render(sf::RenderTarget& target) const override;
    void renderDebug(sf::RenderTarget& target) const;
    void renderVoronoiBoundaries(sf::RenderTarget& target) const;

    
    void renderTerrain(sf::RenderTarget& target, const sf::View& view) const;

    
    void renderPOIs(sf::RenderTarget& target, const sf::View& view) const;

    void setDebugMode(bool enabled) { m_debugMode = enabled; }
    bool isDebugMode() const { return m_debugMode; }

    
    bool loadTerrainAtlas(const std::string& atlasPath);

private:
    
    
    
    sf::IntRect getTerrainTextureRect(MapTile::TerrainType type) const;

    

    
    mutable std::unique_ptr<SpriteComponent> m_sharedSprite;
    bool m_atlasLoaded;

    

    mutable sf::VertexArray m_vertexArray;
    mutable bool m_needsRebuild;

    
    int m_width;                
    int m_height;               
    float m_tileSize;           

    std::vector<MapTile> m_tiles;  
    std::vector<std::unique_ptr<PointOfInterest>> m_pois;

    bool m_debugMode;           
};

#endif