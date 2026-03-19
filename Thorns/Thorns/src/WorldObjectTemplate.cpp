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

    m_shapes.clear();

    for (const auto& layer : mapData.getLayers())
    {
        if (layer->getType() != tmx::Layer::Type::Object) continue;
        if (layer->getName() != "Collision")              continue;

        const auto& group = layer->getLayerAs<tmx::ObjectGroup>();

        for (const auto& obj : group.getObjects())
        {
            WorldObject::Type type;
            if (!nameToType(obj.getName(), type))
            {
                std::cout << "WorldObjectCollisionLoader: Unknown object name '"
                    << obj.getName() << "', skipping.\n";
                continue;
            }

            CollisionShape shape;
            if (!buildShape(obj, shape))
                continue;

            // Append to the type's shape list (one TMX object = one shape)
            m_shapes[type].push_back(std::move(shape));
        }
    }

    for (const auto& [type, shapes] : m_shapes)
    {
        std::cout << "WorldObjectCollisionLoader: Type " << static_cast<int>(type)
            << " -> " << shapes.size() << " collision shape(s)\n";
    }

    return !m_shapes.empty();
}

const std::vector<CollisionShape>* WorldObjectTemplateManager::getShapes(WorldObject::Type type) const
{
    auto it = m_shapes.find(type);
    if (it != m_shapes.end())
        return &it->second;
    return nullptr;
}

bool WorldObjectTemplateManager::hasShapes(WorldObject::Type type) const
{
    return m_shapes.count(type) > 0;
}

bool WorldObjectTemplateManager::nameToType(const std::string& name, WorldObject::Type& outType)
{
    // TMX object name -> WorldObject::Type mapping table.
    // Add rows here when the TMX gains new named objects.
    static const std::pair<const char*, WorldObject::Type> table[] =
    {
        { "Tree_Stump_Large", WorldObject::Type::LargeRoot     },
        { "Tree_Stump_Small", WorldObject::Type::SmallRoot     },
        { "Tree_1",           WorldObject::Type::TreeTop1      },
        { "Tree_2",           WorldObject::Type::TreeTop2      },
        // SmallRootBasic shares the same shape as SmallRoot
        { "Tree_Stump_Small", WorldObject::Type::SmallRootBasic },
    };

    for (const auto& [tmxName, type] : table)
    {
        if (name == tmxName)
        {
            outType = type;
            return true;
        }
    }
    return false;
}

bool WorldObjectTemplateManager::buildShape(const tmx::Object& obj, CollisionShape& outShape)
{
    const auto& aabb = obj.getAABB();
    const float ow = static_cast<float>(aabb.width);
    const float oh = static_cast<float>(aabb.height);

    switch (obj.getShape())
    {
    case tmx::Object::Shape::Rectangle:
    {
        outShape = sf::FloatRect(sf::Vector2f(0.f, 0.f), sf::Vector2f(ow, oh));
        return true;
    }

    case tmx::Object::Shape::Ellipse:
    {
        outShape = sf::FloatRect(sf::Vector2f(0.f, 0.f), sf::Vector2f(ow, oh));
        return true;
    }

    case tmx::Object::Shape::Polygon:
    case tmx::Object::Shape::Polyline:
    {
        CollisionPolygon poly;
        poly.points.reserve(obj.getPoints().size());
        for (const auto& pt : obj.getPoints())
            poly.points.emplace_back(static_cast<float>(pt.x),
                static_cast<float>(pt.y));

        if (poly.points.size() < 3)
            return false;

        outShape = std::move(poly);
        return true;
    }

    default:
        return false;
    }
}



