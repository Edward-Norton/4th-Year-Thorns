#ifndef IPOSITIONABLE_H
#define IPOSITIONABLE_H

#include <SFML/System/Vector2.hpp>

/// <summary>
/// Objects that have a position in world space
/// </summary>
class IPositionable
{
public:
    virtual ~IPositionable() = default;
    virtual sf::Vector2f getPosition() const = 0;
    virtual void setPosition(const sf::Vector2f& pos) = 0;
};

#endif