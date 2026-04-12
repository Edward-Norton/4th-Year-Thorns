#ifndef WORLD_OBJECT_TEMPLATE_H
#define WORLD_OBJECT_TEMPLATE_H

#include <SFML/Graphics.hpp>
#include <tmxlite/Map.hpp>
#include <vector>
#include <string>
#include <unordered_map>
#include <variant>
#include "WorldObject.h"
#include "CollisionType.h"

struct WorldObjectTemplate
{
    std::string name;
    std::vector<CollisionShape> shapes;
};

class WorldObjectTemplateManager
{
public:
    WorldObjectTemplateManager();
    ~WorldObjectTemplateManager() = default;

    
    bool loadTemplates(const std::string& tmxPath);

    const std::vector<CollisionShape>* getShapes(WorldObject::Type type) const;

    bool hasShapes(WorldObject::Type type) const;

private:

    static bool nameToType(const std::string& name, WorldObject::Type& outType);

    static bool buildShape(const tmx::Object& obj, CollisionShape& outShape);

    std::unordered_map<WorldObject::Type, std::vector<CollisionShape>> m_shapes;

};

#endif