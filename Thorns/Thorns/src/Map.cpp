#include "Map.h"
#include <iostream>

Map::Map()
    : m_width(0)
    , m_height(0)
    , m_tileSize(64.f)
    , m_needsRebuild(true)
    , m_debugMode(false)
    , m_atlasLoaded(false)
{
}

void Map::initialize(int width, int height, float tileSize)
{
    m_width = width;
    m_height = height;
    m_tileSize = tileSize;

    // Allocate 2D grid
    m_tiles.clear();
    m_tiles.resize(m_width * m_height);

    m_needsRebuild = true;

    std::cout << "Map initialized: " << m_width << "x" << m_height
        << " tiles (" << getWorldSize().x << "x" << getWorldSize().y << " pixels)\n";
}

void Map::reset()
{
    m_pois.clear();

    // Reset all tiles
    for (auto& tile : m_tiles)
    {
        tile.setTerrainType(MapTile::TerrainType::UNKNOWN);
        tile.setWalkable(false);
        tile.setVoronoiRegion(-1);
    }

    m_needsRebuild = true;
    std::cout << "Map reset: " << m_width << "x" << m_height << " tiles cleared\n";
}

// ========================================================================================================
// QUERY DATA
// ========================================================================================================

MapTile* Map::getTile(int x, int y)
{
    if (!isValidTile(x, y))
        return nullptr;
    return &m_tiles[y * m_width + x];
}

const MapTile* Map::getTile(int x, int y) const
{
    if (!isValidTile(x, y))
        return nullptr;
    return &m_tiles[y * m_width + x];
}

MapTile* Map::getTileAtWorldPos(const sf::Vector2f& worldPos)
{
    sf::Vector2i tileCoords = worldToTile(worldPos);
    return getTile(tileCoords.x, tileCoords.y);
}

const MapTile* Map::getTileAtWorldPos(const sf::Vector2f& worldPos) const
{
    sf::Vector2i tileCoords = worldToTile(worldPos);
    return getTile(tileCoords.x, tileCoords.y);
}

sf::Vector2i Map::worldToTile(const sf::Vector2f& worldPos) const
{
    int x = static_cast<int>(worldPos.x / m_tileSize);
    int y = static_cast<int>(worldPos.y / m_tileSize);
    return sf::Vector2i(x, y);
}

sf::Vector2f Map::tileToWorld(int x, int y) const
{
    float worldX = (x * m_tileSize) + (m_tileSize / 2.f);
    float worldY = (y * m_tileSize) + (m_tileSize / 2.f);
    return sf::Vector2f(worldX, worldY);
}

bool Map::isValidTile(int x, int y) const
{
    return x >= 0 && x < m_width && y >= 0 && y < m_height;
}

sf::Vector2f Map::getWorldSize() const
{
    return sf::Vector2f(m_width * m_tileSize, m_height * m_tileSize);
}

// ========================================================================================================
// POIS
// ========================================================================================================
void Map::addPOI(std::unique_ptr<PointOfInterest> poi)
{
    if (poi)
    {
        std::cout << "Added POI: " << poi->getName() << " at ("
            << poi->getPosition().x << ", " << poi->getPosition().y << ")\n";
        m_pois.push_back(std::move(poi));
        m_needsRebuild = true;
    }
}

bool Map::isInsidePOI(const sf::Vector2f& worldPos) const
{
    for (const auto& poi : m_pois)
    {
        if (poi->contains(worldPos))
            return true;
    }
    return false;
}

void Map::markPOITiles()
{
    // Mark all tiles that fall within POI bounds as POI terrain
    for (const auto& poi : m_pois)
    {
        if (!poi->isBlocking())
            continue;

        sf::FloatRect bounds = poi->getBounds();

        // Get tile range covered by POI
        sf::Vector2i topLeft = worldToTile(sf::Vector2f(bounds.position));
        sf::Vector2i bottomRight = worldToTile(sf::Vector2f(
            bounds.position.x + bounds.size.x,
            bounds.position.y + bounds.size.y
        ));

        // Mark tiles in range
        for (int y = topLeft.y; y <= bottomRight.y; ++y)
        {
            for (int x = topLeft.x; x <= bottomRight.x; ++x)
            {
                MapTile* tile = getTile(x, y);
                if (tile)
                {
                    tile->setTerrainType(MapTile::TerrainType::POI);
                    tile->setWalkable(false);
                }
            }
        }
    }

    m_needsRebuild = true;
}

// ========================================================================================================
// RENDERING
// ========================================================================================================
void Map::render(sf::RenderTarget& target) const
{
    if (m_debugMode)
    {
        renderDebug(target);
    }
    else // Change to sprites later
    {
        renderVisible(target, target.getView());
    }
}

void Map::renderVisible(sf::RenderTarget& target, const sf::View& view) const
{
    if (!m_atlasLoaded || !m_sharedSprite || !m_sharedSprite->isValid())
    {
        renderDebug(target);
        return;
    }

    // Get visible tile bounds
    sf::Vector2f viewCenter = view.getCenter();
    sf::Vector2f viewSize = view.getSize();
    sf::FloatRect viewBounds(
        sf::Vector2f(viewCenter.x - viewSize.x / 2.f, viewCenter.y - viewSize.y / 2.f),
        sf::Vector2f(viewSize.x, viewSize.y)
    );

    sf::Vector2i minTile = worldToTile(sf::Vector2f(viewBounds.position.x, viewBounds.position.y));
    sf::Vector2i maxTile = worldToTile(sf::Vector2f(
        viewBounds.position.x + viewBounds.size.x,
        viewBounds.position.y + viewBounds.size.y
    ));

    // Clamp with padding
    minTile.x = std::max(0, minTile.x - 1);
    minTile.y = std::max(0, minTile.y - 1);
    maxTile.x = std::min(m_width - 1, maxTile.x + 1);
    maxTile.y = std::min(m_height - 1, maxTile.y + 1);

    int tilesRendered = 0;

    // Render visible tiles by repositioning the shared sprite
    for (int y = minTile.y; y <= maxTile.y; ++y)
    {
        for (int x = minTile.x; x <= maxTile.x; ++x)
        {
            const MapTile* tile = getTile(x, y);
            if (!tile)
                continue;

            // Get texture rect for this terrain type
            sf::IntRect texRect = getTerrainTextureRect(tile->getTerrainType());

            // Update shared sprite's texture rect
            m_sharedSprite->setTextureRect(texRect);

            // Position sprite at tile's top-left corner
            float worldX = x * m_tileSize;
            float worldY = y * m_tileSize;
            m_sharedSprite->setPosition(sf::Vector2f(
                std::round(worldX),
                std::round(worldY)
            ));

            // After setting texture rect, verify scale:
            sf::Vector2f spriteSize = m_sharedSprite->getSize();

            // Print once for debugging
            static bool printed = false;
            if (!printed) {
                std::cout << "Sprite rendering size: " << spriteSize.x << "x" << spriteSize.y << "\n";
                std::cout << "Expected tile size: " << m_tileSize << "x" << m_tileSize << "\n";
                printed = true;
            }

            // Render this tile
            m_sharedSprite->render(target);

            ++tilesRendered;
        }
    }

    // Print once per second to avoid spam
    static sf::Clock debugClock;
    if (debugClock.getElapsedTime().asSeconds() > 4.0f)
    {
        std::cout << "Rendering " << tilesRendered << " / "
            << (m_width * m_height) << " tiles ("
            << (tilesRendered * 100.0f / (m_width * m_height))
            << "%)\n";
        debugClock.restart();
    }

    renderVoronoiBoundaries(target);
}

void Map::renderDebug(sf::RenderTarget& target) const
{
    // Rebuild vertex array if needed
    if (m_needsRebuild)
    {
        m_vertexArray.clear();
        m_vertexArray.setPrimitiveType(sf::PrimitiveType::Triangles);
        m_vertexArray.resize(m_width * m_height * 6); // 2 triangles per tile

        for (int y = 0; y < m_height; ++y)
        {
            for (int x = 0; x < m_width; ++x)
            {
                const MapTile* tile = getTile(x, y);
                if (!tile)
                    continue;

                sf::Color color = tile->getDebugColor();

                // Calculate vertex positions
                float left = x * m_tileSize;
                float top = y * m_tileSize;
                float right = left + m_tileSize;
                float bottom = top + m_tileSize;

                // Add tile border by shrinking slightly
                float border = 1.f;
                left += border;
                top += border;
                right -= border;
                bottom -= border;

                // Calculate base index for this tile's vertices
                size_t baseIndex = (y * m_width + x) * 6;

                // First triangle (top-left, top-right, bottom-left)
                m_vertexArray[baseIndex + 0] = sf::Vertex({ sf::Vector2f(left, top), color });
                m_vertexArray[baseIndex + 1] = sf::Vertex({sf::Vector2f(right, top), color});
                m_vertexArray[baseIndex + 2] = sf::Vertex({sf::Vector2f(left, bottom), color});

                // Second triangle (top-right, bottom-right, bottom-left)
                m_vertexArray[baseIndex + 3] = sf::Vertex({ sf::Vector2f(right, top), color });
                m_vertexArray[baseIndex + 4] = sf::Vertex({ sf::Vector2f(right, bottom), color });
                m_vertexArray[baseIndex + 5] = sf::Vertex({ sf::Vector2f(left, bottom), color });
            }
        }

        m_needsRebuild = false;
    }

    // Draw all tiles in one call
    target.draw(m_vertexArray);

    renderVoronoiBoundaries(target);
}

void Map::renderVoronoiBoundaries(sf::RenderTarget& target) const
{
    // Draw lines between tiles that belong to different Voronoi regions
    sf::VertexArray boundaryLines(sf::PrimitiveType::Lines);

    for (int y = 0; y < m_height; ++y)
    {
        for (int x = 0; x < m_width; ++x)
        {
            const MapTile* tile = getTile(x, y);
            if (!tile || tile->getVoronoiRegion() == -1)
                continue;

            int currentRegion = tile->getVoronoiRegion();

            // Check right neighbor
            if (x < m_width - 1)
            {
                const MapTile* rightTile = getTile(x + 1, y);
                if (rightTile && rightTile->getVoronoiRegion() != currentRegion &&
                    rightTile->getVoronoiRegion() != -1)
                {
                    // Draw vertical line on right edge
                    float lineX = (x + 1) * m_tileSize;
                    float lineY1 = y * m_tileSize;
                    float lineY2 = (y + 1) * m_tileSize;

                    boundaryLines.append(sf::Vertex({ sf::Vector2f(lineX, lineY1), sf::Color::White }));
                    boundaryLines.append(sf::Vertex({ sf::Vector2f(lineX, lineY2), sf::Color::White }));
                }
            }

            // Check bottom neighbor
            if (y < m_height - 1)
            {
                const MapTile* bottomTile = getTile(x, y + 1);
                if (bottomTile && bottomTile->getVoronoiRegion() != currentRegion &&
                    bottomTile->getVoronoiRegion() != -1)
                {
                    // Draw horizontal line on bottom edge
                    float lineX1 = x * m_tileSize;
                    float lineX2 = (x + 1) * m_tileSize;
                    float lineY = (y + 1) * m_tileSize;

                    boundaryLines.append(sf::Vertex({ sf::Vector2f(lineX1, lineY), sf::Color::White }));
                    boundaryLines.append(sf::Vertex({ sf::Vector2f(lineX2, lineY), sf::Color::White }));
                }
            }
        }
    }

    target.draw(boundaryLines);
}

// ========================================================================================================
// SPRITE SYSTEM 
// - Was initally having tons of textures for each sprite, made the loading massive
// - Now using ONE sprite for each one.
// ========================================================================================================
bool Map::loadTerrainAtlas(const std::string& atlasPath)
{
    // Create ONE shared sprite component
    m_sharedSprite = std::make_unique<SpriteComponent>();

    // Load the atlas texture into this sprite
    if (!m_sharedSprite->loadTexture(atlasPath, m_tileSize, m_tileSize))
    {
        std::cerr << "Failed to load terrain atlas: " << atlasPath << "\n";
        m_atlasLoaded = false;
        m_sharedSprite.reset();
        m_sharedSprite.reset();
        return false;
    }

    m_atlasLoaded = true;
    std::cout << "Terrain atlas loaded: " << atlasPath << " (using shared sprite)\n";
    return true;
}

sf::IntRect Map::getTerrainTextureRect(MapTile::TerrainType type) const
{
    const int tilePixelSize = 64;
    const int spacing = 1;  // Atlas has 1px spacing
    const int cellSize = tilePixelSize + spacing;

    switch (type)
    {
    case MapTile::TerrainType::Grass:
        return sf::IntRect(sf::Vector2i(cellSize, 0), sf::Vector2i(tilePixelSize, tilePixelSize));

    case MapTile::TerrainType::Forest:
        return sf::IntRect(sf::Vector2i(0, cellSize), sf::Vector2i(tilePixelSize, tilePixelSize));

    case MapTile::TerrainType::DeepForest:
        return sf::IntRect(sf::Vector2i(cellSize, cellSize), sf::Vector2i(tilePixelSize, tilePixelSize));

    case MapTile::TerrainType::POI:
        return sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(tilePixelSize, tilePixelSize));

    default:
        return sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(tilePixelSize, tilePixelSize));
    }
}