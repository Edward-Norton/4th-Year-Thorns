#ifndef IPOSITIONABLE_H
#define IPOSITIONABLE_H

#include <SFML/System/Vector2.hpp>

class IPositionable
{
public:
    virtual ~IPositionable() = default;
    virtual sf::Vector2f getPosition() const = 0;
    virtual void setPosition(const sf::Vector2f& pos) = 0;
};

#endif