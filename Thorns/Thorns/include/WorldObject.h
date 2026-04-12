#ifndef WORLD_OBJECT_HPP
#define WORLD_OBJECT_HPP

#include <SFML/Graphics.hpp>
#include <string>
#include "SpriteComponent.h"
#include "IRenderable.h"
#include "IPositionable.h"
#include "ICollidable.h"
#include "CollisionType.h"

class WorldObject : public IRenderable, public IPositionable, public ICollidable
{
public:
    
    enum class Type
    {
        SmallRoot,      
        TreeTop1,       
        TreeTop2,       
        LargeRoot,      
        SmallRootBasic, 
        COUNT           
    };

    WorldObject(Type type, const sf::Vector2f& worldPos);
    ~WorldObject() = default;

    
    void render(sf::RenderTarget& target) const override;

    
    sf::Vector2f getPosition() const override { return m_worldPosition; }
    void setPosition(const sf::Vector2f& pos) override;

    
    void addCollisionShape(const CollisionShape& shape);
    void clearCollisionShapes();

    
    sf::FloatRect getBounds() const override;

    
    bool loadSpriteFromTexture(const sf::Texture& sharedTexture, const sf::IntRect& textureRect, const sf::Vector2f& size);

    void setCollisionShapes(const std::vector<CollisionShape>* shapes, const sf::Vector2f& templateOrigin);

    bool hasCollisionShapes() const { return m_collisionShapes != nullptr && !m_collisionShapes->empty(); }

    std::vector<CollisionShape> getWorldSpaceShapes() const;

    
    Type getType() const { return m_type; }
    bool isValid() const;

private:
    Type m_type;
    sf::Vector2f m_worldPosition;
    std::unique_ptr<SpriteComponent> m_sprite;

    
    const std::vector<CollisionShape>* m_collisionShapes = nullptr;

    sf::Vector2f m_shapeOffset;
};

#endif