#ifndef POINT_OF_INTEREST_HPP
#define POINT_OF_INTEREST_HPP

#include <SFML/Graphics.hpp>
#include <string>

#include "IRenderable.h"
#include "IPositionable.h"
#include "ICollidable.h"
#include "SpriteComponent.h"
#include "POITemplate.h"

class PointOfInterest : public IRenderable, public IPositionable, public ICollidable
{
public:
    enum class Type
    {
        PlayerHideout,
        
        Village,
        Landmark,
        Farm,
        Quarry,
    };

    PointOfInterest(const std::string& name, Type type, const sf::Vector2f& worldPos, const sf::Vector2f& size);
    ~PointOfInterest() = default;

    
    bool loadSprite(const std::string& spritePath);
    void render(sf::RenderTarget& target) const override;

    
    sf::FloatRect getBounds() const override;
    bool checkEntityCollision(const sf::FloatRect& entityBounds) const;

    
    void addCollisionShape(const CollisionShape& shape);
    void clearCollisionShapes();
    const std::vector<CollisionShape>& getCollisionShapes() const { return m_collisionShapes; }

    
    void addCollisionRect(const sf::FloatRect& rect); 
    void clearCollisionRects();
    std::vector<sf::FloatRect> getCollisionRects() const; 

    
    sf::Vector2f getPosition() const override { return m_worldPosition; }
    void setPosition(const sf::Vector2f& pos) override;

    sf::FloatRect getVisualBounds() const;

    
    
    float getExclusionRadius() const { return m_exclusionRadius; }

    
    bool contains(const sf::Vector2f& worldPos) const;

    
    std::string getName() const { return m_name; }
    Type getType() const { return m_type; }
    bool isBlocking() const { return m_blocking; } 

    sf::Vector2f getSize() const { return m_size; }

    
    bool hasSprite() const;

private:
    std::string m_name;
    Type m_type;

    sf::Vector2f m_worldPosition;  
    sf::Vector2f m_size;            

    float m_exclusionRadius;        
    bool m_blocking;                

    
    std::unique_ptr<SpriteComponent> m_sprite;

    
    
    std::vector<CollisionShape> m_collisionShapes;
};

#endif