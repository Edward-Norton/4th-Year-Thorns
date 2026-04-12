#ifndef IENEMY_H
#define IENEMY_H

#include "IGameEntity.h"
#include <SFML/System/Time.hpp>

class Map;

class IEnemy : public IGameEntity
{
public:
    virtual ~IEnemy() = default;

    
    
    virtual void updateWithContext(sf::Time deltaTime,
        const sf::Vector2f& playerPos,
        const Map* map) = 0;
};

#endif