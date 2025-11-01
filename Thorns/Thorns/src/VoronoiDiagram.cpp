#include "VoronoiDiagram.h"
#include "Map.h"
#include "PointOfInterest.h"
#include "MapTile.h"
#include <iostream>

VoronoiDiagram::VoronoiDiagram()
    : m_map(nullptr)
{
}

void VoronoiDiagram::generate(Map* map, int numSites, unsigned int seed)
{
    if (!map)
    {
        std::cerr << "VoronoiDiagram::generate() - null map pointer, no map passed\n";
        return;
    }

    m_map = map;
    m_sites.clear();

    // Setup random number generator
    std::mt19937 rng(seed == 0 ? std::random_device{}() : seed);

    std::cout << "Generating " << numSites << " Voronoi sites...\n";

    // Step 1: Generate site positions (avoiding POIs)
    generateSites(map, numSites, rng);

    std::cout << "Successfully placed " << m_sites.size() << " sites\n";

    // Step 2: Assign each tile to nearest site
    assignTilesToRegions(map);

    std::cout << "Voronoi diagram generation complete!\n";
}

void VoronoiDiagram::generateSites(Map* map, int numSites, std::mt19937& rng)
{
    sf::Vector2f worldSize = map->getWorldSize();
    const auto& pois = map->getPOIs();

    // Distribution for random positions
    std::uniform_real_distribution<float> distX(0.f, worldSize.x);
    std::uniform_real_distribution<float> distY(0.f, worldSize.y);

    int attempts = 0;
    const int maxAttempts = numSites * 100; // Prevent infinite loop

    while (m_sites.size() < static_cast<size_t>(numSites) && attempts < maxAttempts)
    {
        sf::Vector2f candidatePos(distX(rng), distY(rng));

        // Check if position is valid (not in/near POI)
        if (isValidSitePosition(candidatePos, pois))
        {
            m_sites.emplace_back(candidatePos, static_cast<int>(m_sites.size()));
        }

        ++attempts;
    }

    if (attempts >= maxAttempts)
    {
        std::cerr << "Warning: Could only place " << m_sites.size()
            << " sites after " << maxAttempts << " attempts\n";
    }
}

void VoronoiDiagram::assignTilesToRegions(Map* map)
{
    int width = map->getWidth();
    int height = map->getHeight();

    // For each tile, find nearest Voronoi site
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            MapTile* tile = map->getTile(x, y);
            if (!tile)
                continue;

            // Skip POI tiles - they don't belong to any region
            if (tile->getTerrainType() == MapTile::TerrainType::POI)
                continue;

            // Get world position of tile center
            sf::Vector2f tilePos = map->tileToWorld(x, y);

            // Find closest site
            int closestSiteId = getClosestSiteId(tilePos);

            // Assign tile to that region
            tile->setVoronoiRegion(closestSiteId);
        }
    }
}

bool VoronoiDiagram::isValidSitePosition(const sf::Vector2f& pos,
    const std::vector<std::unique_ptr<PointOfInterest>>& pois) const
{
    // Check against all POIs
    for (const auto& poi : pois)
    {
        // Check if inside POI bounds
        if (poi->contains(pos))
            return false;

        // Check if too close to POI (within exclusion radius)
        float dx = pos.x - poi->getPosition().x;
        float dy = pos.y - poi->getPosition().y;
        float distSq = dx * dx + dy * dy;
        float exclusionRadiusSq = poi->getExclusionRadius() * poi->getExclusionRadius();

        if (distSq < exclusionRadiusSq)
            return false;
    }

    return true;
}

int VoronoiDiagram::getClosestSiteId(const sf::Vector2f& worldPos) const
{
    if (m_sites.empty())
        return -1;

    int closestId = -1;
    float closestDistSq = std::numeric_limits<float>::max();

    for (const auto& site : m_sites)
    {
        float distSq = distanceSquared(worldPos, site.position);

        if (distSq < closestDistSq)
        {
            closestDistSq = distSq;
            closestId = site.regionId;
        }
    }

    return closestId;
}

// Only relative distances are needed, PN sqrt() when I only need the actual distance is needed
float VoronoiDiagram::distanceSquared(const sf::Vector2f& a, const sf::Vector2f& b) const
{
    float dx = b.x - a.x;
    float dy = b.y - a.y;
    return dx * dx + dy * dy;
}

void VoronoiDiagram::renderDebug(sf::RenderTarget& target) const
{
    // Draw sites as circles
    for (const auto& site : m_sites)
    {
        sf::CircleShape circle(8.f);
        circle.setOrigin(sf::Vector2f(8.f, 8.f));
        circle.setPosition(site.position);
        circle.setFillColor(site.debugColor);
        circle.setOutlineColor(sf::Color::White);
        circle.setOutlineThickness(2.f);
        target.draw(circle);
    }
}