#ifndef COLLISION_TYPES_H
#define COLLISION_TYPES_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <variant>

struct CollisionPolygon
{
    std::vector<sf::Vector2f> points;
};

struct CircleShape
{
    std::vector<sf::Vector2f> points;
};

using CollisionShape = std::variant<sf::FloatRect, CollisionPolygon>;

#endif