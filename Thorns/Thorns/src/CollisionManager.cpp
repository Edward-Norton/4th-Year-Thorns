#include "CollisionManager.h"
#include "Map.h"
#include "PointOfInterest.h"
#include "WorldObject.h"
#include <cmath>
#include <algorithm>

CollisionManager::CollisionManager()
{
}

// Basic collision with no account for POI collision params
bool CollisionManager::checkWorldCollision(const sf::FloatRect& entityBounds, const Map* map) const
{
    if (!map)
        return false;

    const auto& pois = map->getPOIs();
    for (const auto& poi : pois)
    {
        if (!poi->isBlocking())
            continue;

        if (poi->checkEntityCollision(entityBounds))
            return true;
    }

    return false;
}

// All collision info, so all collision data stored in unit
CollisionManager::CollisionResult CollisionManager::checkWorldCollisionDetailed(
    const sf::FloatRect& entityBounds, const Map* map) const
{
    CollisionResult result{ false, sf::Vector2f(0.f, 0.f), nullptr };

    if (!map)
        return result;

    //Map pois
    const auto& pois = map->getPOIs();
    for (const auto& poi : pois)
    {
        if (!poi->isBlocking())
            continue;

        // Collision from POITemplate
        const auto& collisionRects = poi->getCollisionRects();
        for (const auto& rect : collisionRects)
        {
            // True if has value
            if (entityBounds.findIntersection(rect).has_value())
            {
                result.collided = true;
                result.penetration = getMinimumTranslationVector(entityBounds, rect); // Shorest vec to seperate the obects
                result.collidedWith = poi.get();
                return result; // Only does first collision, wont do multiple for now
            }
        }
    }

    return result;
}

sf::Vector2f CollisionManager::resolveCollision(const CollisionResult& collision) const
{
    if (!collision.collided)
        return sf::Vector2f(0.f, 0.f);

    return collision.penetration; // MTV
}

// Just to document this for the doc later
// So this is using MTV using SAT (Seperating Axis Theorem
// Due to only being AABBs for now only the X and Y are used
// MTV is the shortest vector to seperate the boxes due to collision overlap
sf::Vector2f CollisionManager::getMinimumTranslationVector(
    const sf::FloatRect& a, const sf::FloatRect& b) const
{
    // Get the centers
    sf::Vector2f aCenter(a.position.x + a.size.x / 2.f, a.position.y + a.size.y / 2.f);
    sf::Vector2f bCenter(b.position.x + b.size.x / 2.f, b.position.y + b.size.y / 2.f);

    // The different
    sf::Vector2f delta = aCenter - bCenter;

    // Overlapping distance
    float overlapX = (a.size.x + b.size.x) / 2.f - std::abs(delta.x);
    float overlapY = (a.size.y + b.size.y) / 2.f - std::abs(delta.y);

    if (overlapX < overlapY)
    {
        return sf::Vector2f(overlapX * (delta.x > 0 ? 1.f : -1.f), 0.f);
    }
    else
    {
        return sf::Vector2f(0.f, overlapY * (delta.y > 0 ? 1.f : -1.f));
    }
}