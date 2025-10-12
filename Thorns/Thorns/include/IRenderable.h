#ifndef IRENDERABLE_H
#define IRENDERABLE_H

#include <SFML/Graphics/RenderTarget.hpp>

/// <summary>
/// Objects that can be drawn to screen
/// </summary>
class IRenderable
{
public:
    virtual ~IRenderable() = default;
    virtual void render(sf::RenderTarget& target) const = 0;
};

#endif