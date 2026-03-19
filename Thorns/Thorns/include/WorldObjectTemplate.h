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

    // Get template by object name string (e.g. "Tree_1")
    const WorldObjectTemplate* getTemplate(const std::string& name) const;

    bool hasTemplate(const std::string& name) const;

    // Apply stored shapes to a WorldObject
    void applyCollision(WorldObject* obj, const std::string& templateName);

private:
    std::unordered_map<std::string, WorldObjectTemplate> m_templates;

    std::vector<CollisionShape> extractShapes(const tmx::Map& mapData, const std::string& targetName);
};

#endif