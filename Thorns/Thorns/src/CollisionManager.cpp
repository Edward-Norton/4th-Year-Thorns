#include "CollisionManager.h"
#include "Map.h"
#include "PointOfInterest.h"
#include "WorldObject.h"
#include <cmath>
#include <algorithm>

CollisionManager::CollisionManager()
{
}

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

CollisionManager::CollisionResult CollisionManager::checkWorldCollisionDetailed(
    const sf::FloatRect& entityBounds, const Map* map) const
{
    CollisionResult result{ false, sf::Vector2f(0.f, 0.f), nullptr };

    if (!map)
        return result;

    const auto& pois = map->getPOIs();
    for (const auto& poi : pois)
    {
        if (!poi->isBlocking())
            continue;

        const auto& collisionRects = poi->getCollisionRects();
        for (const auto& rect : collisionRects)
        {
            if (entityBounds.findIntersection(rect).has_value())
            {
                result.collided = true;
                result.penetration = getMinimumTranslationVector(entityBounds, rect);
                result.collidedWith = poi.get();
                return result;
            }
        }
    }

    return result;
}

sf::Vector2f CollisionManager::resolveCollision(const CollisionResult& collision) const
{
    if (!collision.collided)
        return sf::Vector2f(0.f, 0.f);

    return collision.penetration;
}

sf::Vector2f CollisionManager::getMinimumTranslationVector(
    const sf::FloatRect& a, const sf::FloatRect& b) const
{
    sf::Vector2f aCenter(a.position.x + a.size.x / 2.f, a.position.y + a.size.y / 2.f);
    sf::Vector2f bCenter(b.position.x + b.size.x / 2.f, b.position.y + b.size.y / 2.f);

    sf::Vector2f delta = aCenter - bCenter;

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