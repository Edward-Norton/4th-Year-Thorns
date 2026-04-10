#ifndef COLLISION_TYPES_H
#define COLLISION_TYPES_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <variant>

// Convex polygon defined by a list of points in local or world space
struct CollisionPolygon
{
    std::vector<sf::Vector2f> points;
};

// Circle Collision
struct CircleShape
{
    std::vector<sf::Vector2f> points;
};

using CollisionShape = std::variant<sf::FloatRect, CollisionPolygon>;

#endif