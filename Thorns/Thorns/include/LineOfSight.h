#ifndef LINE_OF_SIGHT_H
#define LINE_OF_SIGHT_H

#include <SFML/System/Vector2.hpp>

class Map;

namespace LineOfSight
{
    bool hasLineOfSight(const sf::Vector2f& from,
        const sf::Vector2f& to,
        const Map* map,
        float maxRange = 600.f);
}

#endif