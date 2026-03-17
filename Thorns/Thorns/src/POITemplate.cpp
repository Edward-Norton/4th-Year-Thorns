#include "POITemplate.h"
#include "PointOfInterest.h"
#include <iostream>
#include <tmxlite/ObjectGroup.hpp>

POITemplateManager::POITemplateManager()
{
}

/// <summary>
// TMX (Tiled Map XML) is an XML-based format from Tiled Map Editor
// Library used: tmxlite (https://github.com/fallahn/tmxlite)
// Also to Note gotten from one of my peers if needed for documentation
/// </summary>
bool POITemplateManager::loadTemplate(const std::string& name, const std::string& tmxPath)
{
    // Creat the tmx map object
    tmx::Map mapData;
    if (!mapData.load(tmxPath))
    {
        std::cerr << "Failed to load POI template: " << tmxPath << "\n";
        return false;
    }

    // Parse and given to template
    POITemplate tmpl = parseTemplate(mapData);
    tmpl.name = name;

    // Store
    m_templates[name] = std::move(tmpl);

    std::cout << "Loaded POI template '" << name << "' with "
        << m_templates[name].shapes.size() << " collision rects\n";

    return true;
}


const POITemplate* POITemplateManager::getTemplate(const std::string& name) const
{
    // Basic it search pointer
    auto it = m_templates.find(name);
    if (it != m_templates.end())
        return &it->second;
    return nullptr;
}

bool POITemplateManager::hasTemplate(const std::string& name) const
{
    return m_templates.find(name) != m_templates.end();
}

// Apply the collision to the POI
void POITemplateManager::applyTemplateCollision(PointOfInterest* poi, const std::string& templateName)
{
    const POITemplate* tmpl = getTemplate(templateName);
    if (!tmpl)
    {
        std::cerr << "Template not found: " << templateName << "\n";
        return;
    }

    // Clear the defaults from constuctor
    poi->clearCollisionRects();

    // Note: Pos is center
    sf::Vector2f poiCenter = poi->getPosition();
    sf::Vector2f poiSize = poi->getSize();

    // Top-left origin of POI in world space — all template coords offset from here
    sf::Vector2f origin(
        poiCenter.x - (poiSize.x / 2.f),
        poiCenter.y - (poiSize.y / 2.f)
    );

    // Transform collision rects from template space to world space
    for (const auto& shape : tmpl->shapes)
    {
        // std::visit selects the correct lambda branch at runtime based on active variant type
        std::visit([&](const auto& s)
        {
                using T = std::decay_t<decltype(s)>;

                if constexpr (std::is_same_v<T, sf::FloatRect>)
                {
                    poi->addCollisionShape(sf::FloatRect(
                        sf::Vector2f(origin.x + s.position.x, origin.y + s.position.y),
                        s.size
                    ));
                }
                else if constexpr (std::is_same_v<T, CollisionPolygon>)
                {
                    CollisionPolygon worldPoly;
                    worldPoly.points.reserve(s.points.size());
                    for (const auto& pt : s.points)
                        worldPoly.points.emplace_back(origin.x + pt.x, origin.y + pt.y);
                    poi->addCollisionShape(std::move(worldPoly));
                }
            }, shape);
    }

    std::cout << "Applied " << tmpl->shapes.size()
        << " collision rects to '" << poi->getName() << "'\n";
}

POITemplate POITemplateManager::parseTemplate(const tmx::Map& mapData)
{
    POITemplate tmpl;

    // Get template size from map
    auto tileSize = mapData.getTileSize();
    auto mapSize = mapData.getTileCount();
    tmpl.size = sf::Vector2f(
        mapSize.x * tileSize.x,
        mapSize.y * tileSize.y
    );

    tmpl.shapes = extractCollisionShapes(mapData);

    return tmpl;
}

std::vector<CollisionShape> POITemplateManager::extractCollisionShapes(const tmx::Map& mapData)
{
    std::vector<CollisionShape> shapes;

    for (const auto& layer : mapData.getLayers())
    {
        if (layer->getType() != tmx::Layer::Type::Object) continue;
        if (layer->getName() != "Collision")              continue;

        const auto& objectGroup = layer->getLayerAs<tmx::ObjectGroup>();
        const auto& layerOffset = objectGroup.getOffset();

        for (const auto& obj : objectGroup.getObjects())
        {
            const auto& aabb = obj.getAABB();
            const float ox = static_cast<float>(aabb.left + layerOffset.x);
            const float oy = static_cast<float>(aabb.top + layerOffset.y);
            const float ow = static_cast<float>(aabb.width);
            const float oh = static_cast<float>(aabb.height);

            switch (obj.getShape())
            {
            case tmx::Object::Shape::Rectangle:
            {
                shapes.emplace_back(sf::FloatRect(
                    sf::Vector2f(ox, oy),
                    sf::Vector2f(ow, oh)
                ));
                break;
            }
            case tmx::Object::Shape::Polygon:
            case tmx::Object::Shape::Polyline:
            {
                CollisionPolygon poly;
                for (const auto& pt : obj.getPoints())
                {
                    // tmxlite polygon points are relative to the object's own position
                    poly.points.emplace_back(ox + pt.x, oy + pt.y);
                }
                if (poly.points.size() >= 3)
                    shapes.emplace_back(std::move(poly));
                break;
            }
            default:
                break;
            }
        }
    }

    return shapes;
}


// Example
/*
To be done
*/