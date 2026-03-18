#ifndef POI_TEMPLATE_H
#define POI_TEMPLATE_H

#include <SFML/Graphics.hpp>
#include <tmxlite/Map.hpp>
#include <vector>
#include <string>
#include <unordered_map>
#include <variant>

class PointOfInterest;

// Convex polygon in template-local space
struct CollisionPolygon
{
    std::vector<sf::Vector2f> points;
};

// Circle Collision
struct
{
    std::vector<sf::Vector2f> points;
};

// Just swap whichever is needed, might be changed to circle for like trees
using CollisionShape = std::variant<sf::FloatRect, CollisionPolygon>;


struct POITemplate
{
    std::string name;
    sf::Vector2f size;
    std::vector<CollisionShape> shapes;
};

class POITemplateManager
{
public:
    POITemplateManager();
    ~POITemplateManager() = default;

    bool loadTemplate(const std::string& name, const std::string& tmxPath);
    const POITemplate* getTemplate(const std::string& name) const;
    bool hasTemplate(const std::string& name) const;
    void applyTemplateCollision(PointOfInterest* poi, const std::string& templateName);

private:
    std::unordered_map<std::string, POITemplate> m_templates;

    POITemplate parseTemplate(const tmx::Map& mapData);
    std::vector<CollisionShape> extractCollisionShapes(const tmx::Map& mapData);
};

#endif