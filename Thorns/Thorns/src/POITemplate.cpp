#include "POITemplate.h"
#include "PointOfInterest.h"
#include <iostream>

POITemplateManager::POITemplateManager()
{
}

bool POITemplateManager::loadTemplate(const std::string& name, const std::string& tmxPath)
{
    tmx::Map mapData;
    if (!mapData.load(tmxPath))
    {
        std::cerr << "Failed to load POI template: " << tmxPath << "\n";
        return false;
    }

    POITemplate tmpl = parseTemplate(mapData);
    tmpl.name = name;

    m_templates[name] = std::move(tmpl);

    std::cout << "Loaded POI template '" << name << "' with "
        << m_templates[name].collisionRects.size() << " collision rects\n";

    return true;
}

const POITemplate* POITemplateManager::getTemplate(const std::string& name) const
{
    auto it = m_templates.find(name);
    if (it != m_templates.end())
        return &it->second;
    return nullptr;
}

bool POITemplateManager::hasTemplate(const std::string& name) const
{
    return m_templates.find(name) != m_templates.end();
}

void POITemplateManager::applyTemplateCollision(PointOfInterest* poi, const std::string& templateName)
{
    const POITemplate* tmpl = getTemplate(templateName);
    if (!tmpl)
    {
        std::cerr << "Template not found: " << templateName << "\n";
        return;
    }

    poi->clearCollisionRects();

    sf::Vector2f poiCenter = poi->getPosition();
    sf::Vector2f poiSize = poi->getSize();

    // Transform collision rects from template space to world space
    for (const auto& rect : tmpl->collisionRects)
    {
        // Template rects are relative to top-left (0,0)
        // Transform to be centered on POI position
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
        if (layer->getType() != tmx::Layer::Type::Object)
            continue;

        if (layer->getName() != "Collision")
            continue;

        const auto& objectGroup = layer->getLayerAs<tmx::ObjectGroup>();
        const auto& objects = objectGroup.getObjects();

        // Get layer offset (critical!)
        const auto& layerOffset = objectGroup.getOffset();

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