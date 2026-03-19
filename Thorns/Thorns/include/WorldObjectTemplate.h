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

// Stores collision shapes for one named object type, in template-local space
struct WorldObjectTemplate
{
    std::string name;
    std::vector<CollisionShape> shapes;
};

// Loads and stores WorldObject collision templates from a TMX file
// Mirrors POITemplateManager for world objects
class WorldObjectTemplateManager
{
public:
    WorldObjectTemplateManager();
    ~WorldObjectTemplateManager() = default;

    // Load all collision shapes from a single TMX file
    bool loadTemplates(const std::string& tmxPath);

    const std::vector<CollisionShape>* getShapes(WorldObject::Type type) const;

    bool hasShapes(WorldObject::Type type) const;

private:

    static bool nameToType(const std::string& name, WorldObject::Type& outType);

    static bool buildShape(const tmx::Object& obj, CollisionShape& outShape);

    std::unordered_map<WorldObject::Type, std::vector<CollisionShape>> m_shapes;

};

#endif