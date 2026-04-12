#include "LineOfSight.h"
#include "Map.h"
#include "MapTile.h"
#include <cmath>

namespace LineOfSight
{
    bool hasLineOfSight(const sf::Vector2f& from,
        const sf::Vector2f& to,
        const Map* map,
        float maxRange)
    {
        if (!map) return false;

        float dx = to.x - from.x;
        float dy = to.y - from.y;
        float dist = std::sqrt(dx * dx + dy * dy);

        if (dist > maxRange) return false;
        if (dist < 1.f)      return true;

        
        float tileSize = map->getTileSize();
        int steps = static_cast<int>(dist / (tileSize * 0.5f)) + 1;

        float stepX = dx / steps;
        float stepY = dy / steps;

        for (int i = 1; i < steps; ++i)
        {
            sf::Vector2f sample(from.x + stepX * i, from.y + stepY * i);
            const MapTile* tile = map->getTileAtWorldPos(sample);

            if (!tile || !tile->isWalkable())
                return false;
        }

        return true;
    }
}