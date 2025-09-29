#ifndef ENEMY_HPP
#define ENEMY_HPP

#include "Entity.h"

class Enemy : public Entity
{
public:
    Enemy();
    ~Enemy() = default;

    // Override the pure virtual functions
    void initTexture() override;
    void update(sf::Time deltaTime) override;
};

#endif