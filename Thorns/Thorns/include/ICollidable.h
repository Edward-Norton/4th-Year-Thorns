#ifndef ICOLLIDABLE_H
#define ICOLLIDABLE_H

#include <SFML/Graphics/Rect.hpp>

/// <summary>
/// Objects that can collide with other objects
/// </summary>
class ICollidable
{
public:
    virtual ~ICollidable() = default;
    virtual sf::FloatRect getBounds() const = 0;
};

#endif