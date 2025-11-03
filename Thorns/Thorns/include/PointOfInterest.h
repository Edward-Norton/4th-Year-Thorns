#ifndef POINT_OF_INTEREST_HPP
#define POINT_OF_INTEREST_HPP

#include <SFML/Graphics.hpp>
#include <string>

/// <summary>
/// Represents a fixed prefab location that blocks procedural generation
/// Examples: Player hideout, key locations
/// 
/// The idea is that this is to block areas in the generation that isnt needed due to being a fixed 
/// asset location
/// 
/// Also needed for the Voronoi diagram due to spacing out these POIs.
/// Some tiles or areas will need to be unwalkable later for ideas down the road.
/// 
/// Main Flow:
/// 1) Default POIs set up so the hideout as its always the same location
/// 2) Map needed to know them before generation
/// 3) Map marks the POIs to prevent generation
/// 4) Algorithims later will prevent generation in the zones
/// 
/// </summary>
class PointOfInterest
{
public:
    enum class Type
    {
        PlayerHideout,
        //Later use ideas
        Village,
        Landmark,
        Farm,
        Quarry,
    };

    PointOfInterest(const std::string& name, Type type, const sf::Vector2f& worldPos, const sf::Vector2f& size);
    ~PointOfInterest() = default;

    // ========== Position & Bounds ==========
    // Get the world-space center position
    sf::Vector2f getPosition() const { return m_worldPosition; }

    // Get axis-aligned bounding box in world space
    sf::FloatRect getBounds() const;

    // Get exclusion radius for Voronoi site placement
    // This prevents Voronoi areas from being placed too close to POI
    float getExclusionRadius() const { return m_exclusionRadius; }

    // Check if a world position is inside this POI
    bool contains(const sf::Vector2f& worldPos) const;

    // ========== Metadata ==========
    std::string getName() const { return m_name; }
    Type getType() const { return m_type; }
    bool isBlocking() const { return m_blocking; } // Needed for the algorithim to skip the tiles

private:
    std::string m_name;
    Type m_type;

    sf::Vector2f m_worldPosition;  // Center position in world space
    sf::Vector2f m_size;            // Width and height in pixels

    float m_exclusionRadius;        // Radius around POI where Voronoi sites can't spawn
    bool m_blocking;                // If true, generation algorithms skip this area
};

#endif