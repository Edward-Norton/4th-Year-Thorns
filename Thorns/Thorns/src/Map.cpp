#include "Map.h"
#include <iostream>

Map::Map()
    : m_width(0)
    , m_height(0)
    , m_tileSize(64.f)
    , m_needsRebuild(true)
{
}

void Map::initialize(int width, int height, float tileSize)
{
    m_width = width;
    m_height = height;
    m_tileSize = tileSize;

    // Allocate 2D grid
    m_tiles.clear();
    m_tiles.resize(m_height);
    for (int y = 0; y < m_height; ++y)
    {
        m_tiles[y].resize(m_width);
    }

    m_needsRebuild = true;

    std::cout << "Map initialized: " << m_width << "x" << m_height
        << " tiles (" << getWorldSize().x << "x" << getWorldSize().y << " pixels)\n";
}

void Map::reset()
{
    // Clear POIs
    m_pois.clear();

    // Reset all tiles to default state
    for (int y = 0; y < m_height; ++y)
    {
        for (int x = 0; x < m_width; ++x)
        {
            MapTile* tile = getTile(x, y);
            if (tile)
            {
                tile->setTerrainType(MapTile::TerrainType::UNKNOWN);
                tile->setWalkable(false);
                tile->setVoronoiRegion(-1);
            }
        }
    }

    m_needsRebuild = true;

    std::cout << "Map reset: " << m_width << "x" << m_height << " tiles cleared\n";
}

MapTile* Map::getTile(int x, int y)
{
    if (!isValidTile(x, y))
        return nullptr;
    return &m_tiles[y][x];
}

const MapTile* Map::getTile(int x, int y) const
{
    if (!isValidTile(x, y))
        return nullptr;
    return &m_tiles[y][x];
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
    // Return center of tile
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

sf::FloatRect Map::getWorldBounds() const
{
    return sf::FloatRect(sf::Vector2f(0.f, 0.f), getWorldSize());
}

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

void Map::render(sf::RenderTarget& target) const
{
    if (m_debugMode)
    {
        renderDebug(target);
    }
    else // Change to sprites later
    {
        renderDebug(target); 
    }
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