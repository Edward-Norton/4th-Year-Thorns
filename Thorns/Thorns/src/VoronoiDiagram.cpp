#include "VoronoiDiagram.h"
#include "Map.h"
#include "PointOfInterest.h"
#include "MapTile.h"
#include <iostream>
#include "MathUtilities.h"

// ========================================================================================================
// SPATIAL GRID IMPLEMENTATION
// ========================================================================================================

SpatialGrid::SpatialGrid()
    : m_cells(),
    m_cellSize(0),
    m_gridHeight(0),
    m_gridWidth(0),
    m_worldHeight(0)
{
}

void SpatialGrid::initialize(float worldWidth, float worldHeight, float cellSize)
{
    m_worldWidth = worldWidth;
    m_worldHeight = worldHeight;
    m_cellSize = cellSize;

    // Calculate grid dimensions (remineder ceil rounds up to nearest whole int)
    m_gridWidth = static_cast<int>(std::ceil(worldWidth / cellSize));
    m_gridHeight = static_cast<int>(std::ceil(worldHeight / cellSize));

    m_cells.clear();

    std::cout << "SpatialGrid initialized: " << m_gridWidth << " × " << m_gridHeight
        << " cells (cell size: " << cellSize << "px)\n";
}

void SpatialGrid::clear()
{
    m_cells.clear();
}

void SpatialGrid::addSite(int siteIndex, const sf::Vector2f& position)
{
    sf::Vector2i gridPos = worldToGrid(position);
    int hash = gridToHash(gridPos.x, gridPos.y);
    m_cells[hash].push_back(siteIndex);
}

std::vector<int> SpatialGrid::getNearbySites(const sf::Vector2f& position) const
{
    std::vector<int> nearbySites;
    sf::Vector2i gridPos = worldToGrid(position);

    // Check 3×3 neighborhood of cells
    for (int dy = -1; dy <= 1; ++dy)
    {
        for (int dx = -1; dx <= 1; ++dx)
        {
            int checkX = gridPos.x + dx;
            int checkY = gridPos.y + dy;

            // Skip out of bounds
            if (checkX < 0 || checkX >= m_gridWidth ||
                checkY < 0 || checkY >= m_gridHeight)
                continue;

            int hash = gridToHash(checkX, checkY);
            auto it = m_cells.find(hash);

            if (it != m_cells.end())
            {
                // Add all sites in this cell
                nearbySites.insert(nearbySites.end(),
                    it->second.begin(),
                    it->second.end());
            }
        }
    }

    return nearbySites;
}

sf::Vector2i SpatialGrid::worldToGrid(const sf::Vector2f& position) const
{
    int x = static_cast<int>(position.x / m_cellSize);
    int y = static_cast<int>(position.y / m_cellSize);

    // Clamp to grid bounds
    x = std::max(0, std::min(x, m_gridWidth - 1));
    y = std::max(0, std::min(y, m_gridHeight - 1));

    return sf::Vector2i(x, y);
}

int SpatialGrid::gridToHash(int gridX, int gridY) const
{
    // Simple hash: treat grid as 1D array
    return gridY * m_gridWidth + gridX;
}

// ========================================================================================================
// VORONOI DIAGRAM IMPLEMENTATION
// ========================================================================================================

VoronoiDiagram::VoronoiDiagram()
    : m_map(nullptr)
{
}

void VoronoiDiagram::generateSitesPoisson(Map* map, unsigned char numSites,
    const sf::Vector2f& hideoutPos,
    float minSiteDistance, std::mt19937& rng)
{
    // Step 1: Clear old data from previous generation
    std::cout << "Generating Voronoi sites with Poisson disk sampling...\n";
    m_sites.clear();
    m_poissonGrid.clear();
    m_activeList.clear();

    // Bridson's Poisson Disk Sampling Algorithm
    // Reference: https://www.cs.ubc.ca/~rbridson/docs/bridson-siggraph07-poissondisk.pdf

    //Step 2: Set up 
    sf::Vector2f worldSize = map->getWorldSize();
    const float hideoutExclusion = 400.f;
    const int k = 30; // Number of attempts per active point

    // Step 3:
    // Cell size for background grid (r / sqrt(2) ensures no two points in same cell)
    float cellSize = minSiteDistance / std::sqrt(2.f);

    // Ensure there are enough cells to cover the world map
    int gridWidth = static_cast<int>(std::ceil(worldSize.x / cellSize));
    int gridHeight = static_cast<int>(std::ceil(worldSize.y / cellSize));

    // Step 4:
    // Background grid for O(1) collision detection
    m_poissonGrid.resize(gridWidth * gridHeight, sf::Vector2f(-1.f, -1.f)); // -1 = empty



    // Distributions
    std::uniform_real_distribution<float> distX(0.f, worldSize.x);
    std::uniform_real_distribution<float> distY(0.f, worldSize.y);
    std::uniform_real_distribution<float> distRadius(minSiteDistance, 2.f * minSiteDistance);
    std::uniform_real_distribution<float> distAngle(0.f, MathUtils::TWO_PI);

    // Step 6: Choose initial sample (avoid hideout)
    sf::Vector2f initialSample;
    int attempts = 0;
    do
    {
        initialSample = sf::Vector2f(distX(rng), distY(rng));
        ++attempts;
    } while (!isValidSitePositionPoisson(initialSample, hideoutPos, minSiteDistance, hideoutExclusion)
        && attempts < 1000);

    if (attempts >= 1000)
    {
        std::cerr << "Failed to find initial Poisson sample\n";
        return;
    }

    // Step 7: Add initial sample
    m_activeList.push_back(initialSample);
    int gridX = static_cast<int>(initialSample.x / cellSize);
    int gridY = static_cast<int>(initialSample.y / cellSize);
    m_poissonGrid[gridY * gridWidth + gridX] = initialSample;

    // Step 8: Generate points from active list
    while (!m_activeList.empty() && m_sites.size() < size_t(numSites))
    {
        // Step 8.1: Pick random point from active list
        std::uniform_int_distribution<size_t> activeDist(0, m_activeList.size() - 1);
        size_t activeIndex = activeDist(rng);
        sf::Vector2f activePoint = m_activeList[activeIndex];

        bool foundValidPoint = false;

        // Step 8.2: Try k times to place a point in the annulus (ring shape)
        // Annulus = area between radius r and 2r from active point
        for (int attempt = 0; attempt < k; ++attempt)
        {
            // Generate point in annulus
            float radius = distRadius(rng); // from the current active point
            float angle = distAngle(rng);

            sf::Vector2f direction = MathUtils::angleRadiansToVector(angle); // No impact on time generation, roughly the same, keeping for readability
            sf::Vector2f candidate = activePoint + (direction * radius);

            // Step 8.3: Check bounds
            if (candidate.x < 0 || candidate.x >= worldSize.x ||
                candidate.y < 0 || candidate.y >= worldSize.y)
                continue;

            // Step 8.4: Check if valid using grid
            int candGridX = static_cast<int>(candidate.x / cellSize);
            int candGridY = static_cast<int>(candidate.y / cellSize);

            bool tooClose = false;

            // Check 5×5 neighborhood (conservative)
            for (int dy = -2; dy <= 2 && !tooClose; ++dy)
            {
                for (int dx = -2; dx <= 2 && !tooClose; ++dx)
                {
                    int checkX = candGridX + dx;
                    int checkY = candGridY + dy;

                    if (checkX < 0 || checkX >= gridWidth ||
                        checkY < 0 || checkY >= gridHeight)
                        continue;

                    sf::Vector2f neighbor = m_poissonGrid[checkY * gridWidth + checkX];

                    // Check if cell is occupied
                    if (neighbor.x >= 0.f)
                    {
                        float distSq = distanceSquared(candidate, neighbor);
                        if (distSq < minSiteDistance * minSiteDistance)
                        {
                            tooClose = true;
                            break;
                        }
                    }
                }
            }

            if (tooClose)
                continue;

            // Step 8.5: Check hideout exclusion
            if (!isValidSitePositionPoisson(candidate, hideoutPos, minSiteDistance, hideoutExclusion))
                continue;

            // Valid point found
            m_activeList.push_back(candidate);
            m_poissonGrid[candGridY * gridWidth + candGridX] = candidate;

            // Create site
            sf::Vector2i tileCoords = map->worldToTile(candidate);
            sf::Vector2f snappedPos = map->tileToWorld(tileCoords.x, tileCoords.y);
            m_sites.emplace_back(snappedPos, tileCoords, static_cast<int>(m_sites.size()));

            foundValidPoint = true;
            break;
        }

        // Step 8.6: If no valid point found after k attempts, remove from active list
        // Had to add to prevent infin loops
        if (!foundValidPoint)
        {
            m_activeList.erase(m_activeList.begin() + activeIndex);
        }
    }

    std::cout << "Poisson disk sampling complete: " << m_sites.size() << " sites generated\n";
}


void VoronoiDiagram::generateSitesRejection(Map* map, unsigned char numSites,
    const sf::Vector2f& hideoutPos,
    float minSiteDistance, std::mt19937& rng)
{

    std::cout << "Generating Voronoi sites with Rejection sampling...\n";
    m_sites.clear();

    sf::Vector2f worldSize = map->getWorldSize();
    const float hideoutExclusion = 400.f;

    std::uniform_real_distribution<float> distX(0.f, worldSize.x);
    std::uniform_real_distribution<float> distY(0.f, worldSize.y);

    int attempts = 0;
    const int maxAttempts = numSites * 1000; // Max 1000 tries per site

    std::cout << "Generating " << static_cast<int>(numSites)
        << " sites using rejection sampling...\n";

    while (m_sites.size() < static_cast<size_t>(numSites) && attempts < maxAttempts)
    {
        // Generate random position
        sf::Vector2f candidatePos(distX(rng), distY(rng));

        // Check distance from hideout
        float distToHideout = std::sqrt(distanceSquared(candidatePos, hideoutPos));
        if (distToHideout < hideoutExclusion)
        {
            ++attempts;
            continue; // Too close to hideout
        }

        // Check distance from existing sites
        bool tooClose = false;
        for (const auto& site : m_sites)
        {
            float distToSite = std::sqrt(distanceSquared(candidatePos, site.position));
            if (distToSite < minSiteDistance)
            {
                tooClose = true;
                break;
            }
        }

        if (tooClose)
        {
            ++attempts;
            continue;
        }

        // Valid position! Snap to tile center
        sf::Vector2i tileCoords = map->worldToTile(candidatePos);
        sf::Vector2f snappedPos = map->tileToWorld(tileCoords.x, tileCoords.y);

        m_sites.emplace_back(snappedPos, tileCoords, static_cast<int>(m_sites.size()));

        ++attempts;
    }

    if (m_sites.size() < static_cast<size_t>(numSites))
    {
        std::cerr << "Warning: Only placed " << m_sites.size()
            << " sites after " << attempts << " attempts\n";
    }
    else
    {
        std::cout << "Rejection sampling complete: " << m_sites.size()
            << " sites placed in " << attempts << " attempts\n";
    }
}

bool VoronoiDiagram::isValidSitePositionPoisson(const sf::Vector2f& pos,
    const sf::Vector2f& hideoutPos,
    float minDist, float hideoutExclusion) const
{
    // Check distance from hideout
    float distToHideout = std::sqrt(distanceSquared(pos, hideoutPos));
    return distToHideout >= hideoutExclusion;
}

void VoronoiDiagram::assignTilesToRegionsSP(Map* map)
{
    int width = map->getWidth();
    int height = map->getHeight();
    int tilesProcessed = 0;

    std::cout << "Assigning " << (width * height) << " tiles to Voronoi regions...\n";

    // For each tile, find nearest site using spatial grid
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            MapTile* tile = map->getTile(x, y);
            if (!tile)
                continue;

            // Skip POI tiles
            if (tile->getTerrainType() == MapTile::TerrainType::POI)
                continue;

            // Get world position of tile center
            sf::Vector2f tilePos = map->tileToWorld(x, y);

            // Get nearby sites from spatial grid (O(k) instead of O(s))
            std::vector<int> nearbySiteIndices = m_spatialGrid.getNearbySites(tilePos);

            if (nearbySiteIndices.empty())
            {
                // Fallback: shouldn't happen but check all sites if grid fails
                tile->setVoronoiRegion(getClosestSiteId(tilePos));
            }
            else
            {
                // Find closest among nearby sites
                int closestId = -1;
                float closestDistSq = std::numeric_limits<float>::max();

                for (int siteIdx : nearbySiteIndices)
                {
                    const VoronoiSite& site = m_sites[siteIdx];
                    float distSq = distanceSquared(tilePos, site.position);

                    if (distSq < closestDistSq)
                    {
                        closestDistSq = distSq;
                        closestId = site.regionId;
                    }
                }

                tile->setVoronoiRegion(closestId);
            }

            ++tilesProcessed;
        }
    }

    std::cout << "Tile assignment complete: " << tilesProcessed << " tiles assigned\n";
}

bool VoronoiDiagram::isValidSitePosition(const sf::Vector2f& pos, const sf::Vector2f& hideoutPos,
    float minSiteDistance, float hideoutExclusion) const
{
    // Check distance from hideout
    float distToHideout = std::sqrt(distanceSquared(pos, hideoutPos));
    if (distToHideout < hideoutExclusion)
        return false;

    // Check distance from all existing sites
    for (const auto& site : m_sites)
    {
        float distToSite = std::sqrt(distanceSquared(pos, site.position));
        if (distToSite < minSiteDistance)
            return false;
    }

    return true;
}

void VoronoiDiagram::markSiteWithPOI(int siteID)
{

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



// ========================================================================================================
// RANDOM NUMBER GENERATION - TECHNICAL NOTES
// ========================================================================================================
/*
* Video References and Notes:
* "Stop using rand()" - https://youtu.be/oW6iuFbwPDg?si=osEtvERGpgz_uBYN
* "std::mt19937" - https://www.geeksforgeeks.org/cpp/stdmt19937-class-in-cpp/
*
* 1. PRNG - std::mt19937
* -----------------------
* - Same seed same sequence
*
* used to save the seed and get the same sequence for when saving is added, wanted to future proof this aspect now
*
* 2. True Random on hardware - std::random_device
* --------------------------
* Hardware based source
* No seed so cant reproduce or save sequence
* Mainy for fallback here and wanted to test it
*
*======================================================================================================== */
