#include "CollisionManager.h"
#include "Map.h"
#include "PointOfInterest.h"
#include "WorldObject.h"
#include <cmath>
#include <algorithm>

static constexpr int MAX_SHAPES_TO_TEST = 3;

CollisionManager::CollisionManager()
{
}

bool CollisionManager::checkWorldCollision(const sf::FloatRect& entityBounds, const Map* map) const
{
    return false;
}

CollisionManager::CollisionResult CollisionManager::checkWorldCollisionDetailed(
    const sf::FloatRect& entityBounds, const Map* map) const
{
    CollisionResult result{ false, sf::Vector2f(0.f, 0.f), nullptr };
    if (!map) return result;

    sf::Vector2f entityCenter(
        entityBounds.position.x + entityBounds.size.x / 2.f,
        entityBounds.position.y + entityBounds.size.y / 2.f
    );

    for (const auto& poi : map->getPOIs())
    {
        if (!poi->isBlocking()) continue;

        
        if (!entityBounds.findIntersection(poi->getBounds()).has_value())
            continue;

        const auto& shapes = poi->getCollisionShapes();
        if (shapes.empty()) continue;

        
        const auto rects = poi->getCollisionRects();
        std::vector<std::pair<float, size_t>> shapeDists;
        shapeDists.reserve(shapes.size());

        for (size_t i = 0; i < rects.size(); ++i)
        {
            sf::Vector2f shapeCenter(
                rects[i].position.x + rects[i].size.x / 2.f,
                rects[i].position.y + rects[i].size.y / 2.f
            );
            float dx = entityCenter.x - shapeCenter.x;
            float dy = entityCenter.y - shapeCenter.y;
            shapeDists.emplace_back(dx * dx + dy * dy, i);
        }

        
        int testCount = std::min(static_cast<int>(shapeDists.size()), MAX_SHAPES_TO_TEST);
        std::partial_sort(shapeDists.begin(), shapeDists.begin() + testCount,
            shapeDists.end(),
            [](const auto& a, const auto& b) { return a.first < b.first; });

        
        for (int i = 0; i < testCount; ++i)
        {
            size_t idx = shapeDists[i].second;

            bool hit = std::visit([&](const auto& s) -> bool
            {
                    using T = std::decay_t<decltype(s)>;
                    if constexpr (std::is_same_v<T, sf::FloatRect>)
                        return entityBounds.findIntersection(s).has_value();
                    else if constexpr (std::is_same_v<T, CollisionPolygon>)
                        return aabbVsPolygon(entityBounds, s.points);
                    return false;
            }, shapes[idx]);

            if (hit)
            {
                result.collided = true;
                result.collidedWith = poi.get();

                bool isPoly = std::holds_alternative<CollisionPolygon>(shapes[idx]);
                if (isPoly)
                    result.penetration = getMTVPolygon(entityBounds,
                        std::get<CollisionPolygon>(shapes[idx]).points);
                else
                    result.penetration = getMinimumTranslationVector(entityBounds, rects[idx]);

                return result;
            }
        }
    }

    return result;
}

sf::Vector2f CollisionManager::resolveCollision(const CollisionResult& collision) const
{
    if (!collision.collided)
        return sf::Vector2f(0.f, 0.f);

    return collision.penetration; 
}

bool CollisionManager::aabbVsPolygon(const sf::FloatRect& box,
    const std::vector<sf::Vector2f>& points)
{
    if (points.size() < 3) return false;

    const sf::Vector2f boxCorners[4] = {
        { box.position.x,              box.position.y              },
        { box.position.x + box.size.x, box.position.y              },
        { box.position.x + box.size.x, box.position.y + box.size.y },
        { box.position.x,              box.position.y + box.size.y }
    };

    auto project = [](const sf::Vector2f* pts, int count, const sf::Vector2f& axis)
        -> std::pair<float, float>
        {
            float minP = std::numeric_limits<float>::max();
            float maxP = -std::numeric_limits<float>::max();
            for (int i = 0; i < count; ++i)
            {
                float d = pts[i].x * axis.x + pts[i].y * axis.y;
                minP = std::min(minP, d);
                maxP = std::max(maxP, d);
            }
            return { minP, maxP };
        };

    
    std::vector<sf::Vector2f> axes;
    axes.reserve(2 + points.size());
    axes.emplace_back(1.f, 0.f);
    axes.emplace_back(0.f, 1.f);

    for (size_t i = 0; i < points.size(); ++i)
    {
        const sf::Vector2f& a = points[i];
        const sf::Vector2f& b = points[(i + 1) % points.size()];
        sf::Vector2f edge(b.x - a.x, b.y - a.y);
        sf::Vector2f normal(-edge.y, edge.x);
        float len = std::sqrt(normal.x * normal.x + normal.y * normal.y);
        if (len > 0.f)
            axes.emplace_back(normal.x / len, normal.y / len);
    }

    for (const auto& axis : axes)
    {
        auto [boxMin, boxMax] = project(boxCorners, 4, axis);
        auto [polyMin, polyMax] = project(points.data(), static_cast<int>(points.size()), axis);

        if (boxMax < polyMin || polyMax < boxMin)
            return false; 
    }

    return true; 
}

sf::Vector2f CollisionManager::getMinimumTranslationVector(
    const sf::FloatRect& a, const sf::FloatRect& b) const
{
    
    sf::Vector2f aCenter(a.position.x + a.size.x / 2.f, a.position.y + a.size.y / 2.f);
    sf::Vector2f bCenter(b.position.x + b.size.x / 2.f, b.position.y + b.size.y / 2.f);

    
    sf::Vector2f delta = aCenter - bCenter;

    
    float overlapX = (a.size.x + b.size.x) / 2.f - std::abs(delta.x);
    float overlapY = (a.size.y + b.size.y) / 2.f - std::abs(delta.y);

    if (overlapX < overlapY)
    {
        return sf::Vector2f(overlapX * (delta.x > 0 ? 1.f : -1.f), 0.f);
    }
    else
    {
        return sf::Vector2f(0.f, overlapY * (delta.y > 0 ? 1.f : -1.f));
    }
}

sf::Vector2f CollisionManager::getMTVPolygon(const sf::FloatRect& box,
    const std::vector<sf::Vector2f>& points) const
{
    if (points.size() < 3) return sf::Vector2f(0.f, 0.f);

    const sf::Vector2f boxCorners[4] = {
        { box.position.x,              box.position.y              },
        { box.position.x + box.size.x, box.position.y              },
        { box.position.x + box.size.x, box.position.y + box.size.y },
        { box.position.x,              box.position.y + box.size.y }
    };

    auto project = [](const sf::Vector2f* pts, int count, const sf::Vector2f& axis)
        -> std::pair<float, float>
        {
            float minP = std::numeric_limits<float>::max();
            float maxP = -std::numeric_limits<float>::max();
            for (int i = 0; i < count; ++i)
            {
                float d = pts[i].x * axis.x + pts[i].y * axis.y;
                minP = std::min(minP, d);
                maxP = std::max(maxP, d);
            }
            return { minP, maxP };
        };

    std::vector<sf::Vector2f> axes;
    axes.reserve(2 + points.size());
    axes.emplace_back(1.f, 0.f);
    axes.emplace_back(0.f, 1.f);

    for (size_t i = 0; i < points.size(); ++i)
    {
        sf::Vector2f edge(
            points[(i + 1) % points.size()].x - points[i].x,
            points[(i + 1) % points.size()].y - points[i].y
        );
        sf::Vector2f normal(-edge.y, edge.x);
        float len = std::sqrt(normal.x * normal.x + normal.y * normal.y);
        if (len > 0.f)
            axes.emplace_back(normal.x / len, normal.y / len);
    }

    float minOverlap = std::numeric_limits<float>::max();
    sf::Vector2f mtv;

    sf::Vector2f boxCenter(box.position.x + box.size.x / 2.f, box.position.y + box.size.y / 2.f);
    sf::Vector2f polyCentroid;
    for (const auto& pt : points) polyCentroid += pt;
    polyCentroid /= static_cast<float>(points.size());

    for (const auto& axis : axes)
    {
        auto [boxMin, boxMax] = project(boxCorners, 4, axis);
        auto [polyMin, polyMax] = project(points.data(), static_cast<int>(points.size()), axis);

        float overlap = std::min(boxMax, polyMax) - std::max(boxMin, polyMin);
        if (overlap < 0.f) return sf::Vector2f(0.f, 0.f);

        if (overlap < minOverlap)
        {
            minOverlap = overlap;
            mtv = axis * overlap;

            
            sf::Vector2f dir = boxCenter - polyCentroid;
            if ((dir.x * mtv.x + dir.y * mtv.y) < 0.f)
                mtv = -mtv;
        }
    }

    return mtv;
}

CollisionManager::CollisionResult CollisionManager::checkCollisionWith(
    const sf::FloatRect& entityBounds,
    const std::vector<std::unique_ptr<WorldObject>>& objects) const
{
    CollisionResult result{ false, sf::Vector2f(0.f, 0.f), nullptr };

    for (const auto& obj : objects)
    {
        
        if (!entityBounds.findIntersection(obj->getBounds()).has_value())
            continue;

        if (obj->hasCollisionShapes())
        {
            const auto worldShapes = obj->getWorldSpaceShapes();

            for (const auto& shape : worldShapes)
            {
                bool hit = std::visit([&](const auto& s) -> bool
                    {
                        using T = std::decay_t<decltype(s)>;
                        if constexpr (std::is_same_v<T, sf::FloatRect>)
                            return entityBounds.findIntersection(s).has_value();
                        else if constexpr (std::is_same_v<T, CollisionPolygon>)
                            return aabbVsPolygon(entityBounds, s.points);
                        return false;
                    }, shape);

                if (hit)
                {
                    result.collided = true;
                    result.collidedWith = obj.get();

                    std::visit([&](const auto& s)
                        {
                            using T = std::decay_t<decltype(s)>;
                            if constexpr (std::is_same_v<T, sf::FloatRect>)
                                result.penetration = getMinimumTranslationVector(entityBounds, s);
                            else if constexpr (std::is_same_v<T, CollisionPolygon>)
                                result.penetration = getMTVPolygon(entityBounds, s.points);
                        }, shape);

                    return result;
                }
            }
        }
        else
        {
            
            result.collided = true;
            result.penetration = getMinimumTranslationVector(entityBounds, obj->getBounds());
            result.collidedWith = obj.get();
            return result;
        }
    }

    return result;
}
