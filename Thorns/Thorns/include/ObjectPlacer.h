#ifndef OBJECT_PLACER_HPP
#define OBJECT_PLACER_HPP

#include <memory>
#include <vector>
#include <unordered_map>
#include <string>
#include <SFML/Graphics.hpp>
#include "PerlinNoise.h"
#include "WorldObject.h"

class Map;

/// <summary>
/// Manages procedural object placement using Perlin noise
/// 
/// Strategy:
/// 1. Generate Perlin noise value for each tile
/// 2. Apply threshold to determine if object should be placed
/// 3. Use noise value to select object variant
/// 4. Respect terrain types and POI exclusions
/// 
/// Performance:
/// - Objects generated during map creation (not runtime)
/// - Frustum culling during rendering
/// - Shared texture atlas for all objects
/// </summary>
class ObjectPlacer
{
public:
    /// Configuration for object placement
    struct PlacementSettings
    {
        // ========== Noise Parameters ==========
        double frequency = 0.1;          // Lower = larger patterns, Higher = smaller patterns
        int octaves = 2;                 // Number of noise layers (more = more detail)
        double persistence = 0.5;        // Amplitude decrease per octave

        // ========== Placement Rules ==========
        double placementThreshold = 0.6; // Only place if noise > threshold
        double spacing = 32.0;           // Minimum distance between objects (pixels)

        // ========== Object Type ==========
        WorldObject::Type objectType = WorldObject::Type::SmallRoot;

        // ========== Filtering ==========
        bool respectPOIs = true;         // Don't place objects in POI areas
        bool grassOnly = true;           // Only place on grass terrain
    };

    ObjectPlacer();
    ~ObjectPlacer() = default;

    // ========== Initialization ==========

    /// Load object atlas and parse object definitions
    bool initialize(const std::string& atlasPath, const std::string& definitionsPath);

    // ========== Object Generation ==========

    /// Generate objects for entire map using Perlin noise
    /// @param map - Map to place objects on
    /// @param settings - Placement configuration
    /// @param seed - Random seed for reproducible placement
    void generateObjects(Map* map, const PlacementSettings& settings, unsigned int seed);

    /// Clear all placed objects
    void clearObjects();

    // ========== Rendering ==========

    /// Render visible objects (with frustum culling)
    void render(sf::RenderTarget& target, const sf::View& view) const;

    /// Render debug visualization (noise values, placement points)
    void renderDebug(sf::RenderTarget& target, const sf::View& view) const;

    // ========== Queries ==========

    int getObjectCount() const { return m_objects.size(); }
    const std::vector<std::unique_ptr<WorldObject>>& getObjects() const { return m_objects; }

private:
    // ========== Object Definitions ==========

    /// Information about an object type from atlas
    struct ObjectDefinition
    {
        std::string name;
        sf::IntRect textureRect;  // Position in atlas
        sf::Vector2f size;        // Render size in world
    };

    // ========== Helper Methods ==========

    /// Parse object definitions from text file
    bool parseDefinitions(const std::string& definitionsPath);

    /// Check if position is valid for object placement
    bool isValidPlacement(const sf::Vector2f& worldPos, Map* map, const PlacementSettings& settings) const;

    /// Get object definition for a type
    const ObjectDefinition* getDefinition(WorldObject::Type type) const;

    // ========== Data ==========

    std::unique_ptr<PerlinNoise> m_perlin;
    std::vector<std::unique_ptr<WorldObject>> m_objects;

    // Object atlas
    std::string m_atlasPath;
    std::unordered_map<WorldObject::Type, ObjectDefinition> m_definitions;

    bool m_initialized;
};

#endif

// ========================================================================================================
// USAGE EXAMPLE
// ========================================================================================================
/*
 * To be worked on
 */