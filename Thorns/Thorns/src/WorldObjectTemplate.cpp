#include "WorldObjectTemplate.h"
#include <tmxlite/ObjectGroup.hpp>
#include <iostream>

WorldObjectTemplateManager::WorldObjectTemplateManager()
{
}

bool WorldObjectTemplateManager::loadTemplates(const std::string& tmxPath)
{
    tmx::Map mapData;
    if (!mapData.load(tmxPath))
    {
        std::cerr << "WorldObjectTemplateManager: Failed to load: " << tmxPath << "\n";
        return false;
    }

    // Find the Collision object group
    for (const auto& layer : mapData.getLayers())
    {
        if (layer->getType() != tmx::Layer::Type::Object) continue;
        if (layer->getName() != "Collision")              continue;

        const auto& objectGroup = layer->getLayerAs<tmx::ObjectGroup>();

        for (const auto& obj : objectGroup.getObjects())
        {
            std::string name = obj.getName();
            if (name.empty()) continue;

            WorldObjectTemplate tmpl;
            tmpl.name = name;

            const auto& aabb = obj.getAABB();
            const float ox = static_cast<float>(aabb.left);
            const float oy = static_cast<float>(aabb.top);
            const float ow = static_cast<float>(aabb.width);
            const float oh = static_cast<float>(aabb.height);

            switch (obj.getShape())
            {
            case tmx::Object::Shape::Rectangle:
            {
                // V1 BUG: stored in template-local space, never offset to world pos
                tmpl.shapes.emplace_back(sf::FloatRect(
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
                    poly.points.emplace_back(ox + pt.x, oy + pt.y);
                if (poly.points.size() >= 3)
                    tmpl.shapes.emplace_back(std::move(poly));
                break;
            }
            default:
                // V1: ellipses silently dropped
                break;
            }

            if (!tmpl.shapes.empty())
                m_templates[name] = std::move(tmpl);
        }
    }

    std::cout << "WorldObjectTemplateManager: Loaded " << m_templates.size() << " templates\n";
    return !m_templates.empty();
}

const WorldObjectTemplate* WorldObjectTemplateManager::getTemplate(const std::string& name) const
{
    auto it = m_templates.find(name);
    return it != m_templates.end() ? &it->second : nullptr;
}

bool WorldObjectTemplateManager::hasTemplate(const std::string& name) const
{
    return m_templates.find(name) != m_templates.end();
}

void WorldObjectTemplateManager::applyCollision(WorldObject* obj, const std::string& templateName)
{
    const WorldObjectTemplate* tmpl = getTemplate(templateName);
    if (!tmpl)
    {
        std::cerr << "WorldObjectTemplateManager: Template not found: " << templateName << "\n";
        return;
    }

    obj->clearCollisionShapes();

    // V1 BUG: shapes are in template-local space, not offset to obj->getPosition()
    for (const auto& shape : tmpl->shapes)
        obj->addCollisionShape(shape);
}