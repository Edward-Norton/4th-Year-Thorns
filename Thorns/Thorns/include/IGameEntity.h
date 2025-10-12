#ifndef IGAME_ENTITY_H
#define IGAME_ENTITY_H

#include "IUpdatable.h"
#include "IRenderable.h"
#include "IPositionable.h"
#include "ICollidable.h"

/// <summary>
/// Most game entities will need these interfaces
/// </summary>
class IGameEntity : public IUpdatable,
    public IRenderable,
    public IPositionable,
    public ICollidable
{
public:
    virtual ~IGameEntity() = default;

    virtual bool isActive() const = 0;
    virtual void setActive(bool active) = 0;
};

#endif