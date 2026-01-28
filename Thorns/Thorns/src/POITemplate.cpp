#include "POITemplate.h"
#include "PointOfInterest.h"
#include <iostream>

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
        << m_templates[name].collisionRects.size() << " collision rects\n";

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

    // Transform collision rects from template space to world space
    for (const auto& rect : tmpl->collisionRects)
    {
        // Template rects are relative to top-left (0,0)
        // Transform to be centered on POI position so:
        // 1. Subtract half size to get top-left of POI
        // 2. Add template rect position
        sf::FloatRect worldRect(
            sf::Vector2f(
                poiCenter.x - (poiSize.x / 2.f) + rect.position.x,
                poiCenter.y - (poiSize.y / 2.f) + rect.position.y
            ),
            rect.size
        );

        poi->addCollisionRect(worldRect);
    }

    std::cout << "Applied " << tmpl->collisionRects.size()
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

    tmpl.collisionRects = extractCollisionRects(mapData);

    return tmpl;
}

std::vector<sf::FloatRect> POITemplateManager::extractCollisionRects(const tmx::Map& mapData)
{
    std::vector<sf::FloatRect> rects;

    for (const auto& layer : mapData.getLayers())
    {
        // Step 1: Skip non-object layers, due to how I used tiled, only objects for collisions
        if (layer->getType() != tmx::Layer::Type::Object)
            continue;

        // Step 2: Only parse Collision
        if (layer->getName() != "Collision")
            continue;

        // Step 3: Cast to object group to access objects
        const auto& objectGroup = layer->getLayerAs<tmx::ObjectGroup>();
        const auto& objects = objectGroup.getObjects();

        // Get layer offset. Note: Needed to due how the .tmx stores the data
        const auto& layerOffset = objectGroup.getOffset();

        // Step 5: Extract each object as collision rectangle
        for (const auto& obj : objects)
        {
            auto aabb = obj.getAABB();

            rects.push_back(
                sf::FloatRect(
                    sf::Vector2f(
                        static_cast<float>(aabb.left + layerOffset.x),  // Add offset X
                        static_cast<float>(aabb.top + layerOffset.y)    // Add offset Y
                    ),
                    sf::Vector2f(
                        static_cast<float>(aabb.width),
                        static_cast<float>(aabb.height)
                    )
                )
            );
        }
    }

    return rects;
}


// Example
/*
To be done
*/