#ifndef IUPDATABLE_H
#define IUPDATABLE_H

#include <SFML/System/Time.hpp>

class IUpdatable
{
public:
    virtual ~IUpdatable() = default;
    virtual void update(sf::Time deltaTime) = 0;
};

#endif