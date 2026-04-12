#ifndef COLLISION_MANAGER_H
#define COLLISION_MANAGER_H

#include <SFML/Graphics.hpp>
#include <vector>
#include "ICollidable.h"

class Map;
class WorldObject;

class CollisionManager
{
public:
    CollisionManager();
    ~CollisionManager() = default;

    
    struct CollisionResult
    {
        bool collided;
        sf::Vector2f penetration;
        const ICollidable* collidedWith;
    };

    
    bool checkWorldCollision(const sf::FloatRect& entityBounds, const Map* map) const;

    
    CollisionResult checkWorldCollisionDetailed(const sf::FloatRect& entityBounds, const Map* map) const;

    
    template<typename T>
    CollisionResult checkCollisionWith(const sf::FloatRect& entityBounds,
        const std::vector<std::unique_ptr<T>>& objects) const;

    CollisionResult checkCollisionWith(const sf::FloatRect& entityBounds,
        const std::vector<std::unique_ptr<WorldObject>>& objects) const;

    
    sf::Vector2f resolveCollision(const CollisionResult& collision) const;

    
    static bool aabbVsPolygon(const sf::FloatRect& box, const std::vector<sf::Vector2f>& points);

private:
    
    sf::Vector2f getMinimumTranslationVector(const sf::FloatRect& a, const sf::FloatRect& b) const;

    
    sf::Vector2f getMTVPolygon(const sf::FloatRect& box, const std::vector<sf::Vector2f>& points) const;
};

template<typename T>
CollisionManager::CollisionResult CollisionManager::checkCollisionWith(
    const sf::FloatRect& entityBounds,
    const std::vector<std::unique_ptr<T>>& objects) const
{
    CollisionResult result{ false, sf::Vector2f(0.f, 0.f), nullptr };

    for (const auto& obj : objects)
    {
        sf::FloatRect objBounds = obj->getBounds();

        if (entityBounds.findIntersection(objBounds).has_value())
        {
            result.collided = true;
            result.penetration = getMinimumTranslationVector(entityBounds, objBounds);
            result.collidedWith = obj.get();
            return result;
        }
    }

    return result;
}

#endif