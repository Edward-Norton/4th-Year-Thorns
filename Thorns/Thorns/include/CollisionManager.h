#ifndef COLLISION_MANAGER_H
#define COLLISION_MANAGER_H

#include <SFML/Graphics.hpp>
#include <vector>
#include "ICollidable.h"

class Map;

/// <summary>
/// Generic collision detection and resolution system
/// Handles collision between any ICollidable objects
/// Separates collision logic from gameplay code for reusability
/// </summary>
class CollisionManager
{
public:
    CollisionManager();
    ~CollisionManager() = default;

    // Collision result with detailed information
    struct CollisionResult
    {
        bool collided;
        sf::Vector2f penetration;
        const ICollidable* collidedWith;
    };

    // Check if entity collides with any world object
    bool checkWorldCollision(const sf::FloatRect& entityBounds, const Map* map) const;

    // Check collision with world and return detailed result
    CollisionResult checkWorldCollisionDetailed(const sf::FloatRect& entityBounds, const Map* map) const;

    // Check collision against a collection of collidable objects
    template<typename T>
    CollisionResult checkCollisionWith(const sf::FloatRect& entityBounds,
        const std::vector<std::unique_ptr<T>>& objects) const;

    // Resolve collision by calculating correction vector
    sf::Vector2f resolveCollision(const CollisionResult& collision) const;

private:
    // Calculate minimum translation vector to separate two rectangles
    sf::Vector2f getMinimumTranslationVector(const sf::FloatRect& a, const sf::FloatRect& b) const;
};

// Template implementation for generic collision checking
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