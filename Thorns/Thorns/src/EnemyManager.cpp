#include "EnemyManager.h"
#include "CollisionManager.h"
#include "Map.h"
#include <iostream>

EnemyManager::EnemyManager()
    : m_initialized(false)
{
}

// Initialization
bool EnemyManager::initialize(const std::string& savageAtlasPath,
    const std::string& chomperAtlasPath)
{
    // Pre-warm every pool slot so textures are loaded once
    for (auto& enemy : m_savagePool)
    {
        if (!enemy.initialize(savageAtlasPath))
        {
            std::cerr << "EnemyManager: Failed to initialize SavageEnemy pool slot\n";
            return false;
        }
        enemy.setActive(false);  // All slots start inactive
    }

    for (auto& enemy : m_chomperPool)
    {
        if (!enemy.initialize(chomperAtlasPath))
        {
            std::cerr << "EnemyManager: Failed to initialize ChomperEnemy pool slot\n";
            return false;
        }
        enemy.setActive(false);
    }

    m_initialized = true;
    std::cout << "EnemyManager: Pools ready ("
        << MAX_SAVAGE << " savage, "
        << MAX_CHOMPER << " chomper slots)\n";
    return true;
}

// ========================================
// Spawning
// ========================================
SavageEnemy* EnemyManager::spawnSavage(const sf::Vector2f& worldPos)
{
    for (auto& enemy : m_savagePool)
    {
        if (!enemy.isActive())
        {
            enemy.setPosition(worldPos);
            enemy.setActive(true);
            return &enemy;
        }
    }
    std::cerr << "EnemyManager::spawnSavage: Pool exhausted\n";
    return nullptr;
}

ChomperEnemy* EnemyManager::spawnChomper(const sf::Vector2f& worldPos)
{
    for (auto& enemy : m_chomperPool)
    {
        if (!enemy.isActive())
        {
            enemy.setPosition(worldPos);
            enemy.setActive(true);
            return &enemy;
        }
    }
    std::cerr << "EnemyManager::spawnChomper: Pool exhausted\n";
    return nullptr;
}

void EnemyManager::despawnSavage(SavageEnemy* enemy)
{
    if (enemy) enemy->setActive(false);
}

void EnemyManager::despawnChomper(ChomperEnemy* enemy)
{
    if (enemy) enemy->setActive(false);
}

void EnemyManager::despawnAll()
{
    for (auto& e : m_savagePool)  e.setActive(false);
    for (auto& e : m_chomperPool) e.setActive(false);
}

// ========================================
// Per-frame update
// Order: AI + movement first, then collision so enemies never rest inside geometry
// ========================================
void EnemyManager::updateAll(sf::Time deltaTime,
    const sf::Vector2f& playerPos,
    const Map* map,
    const CollisionManager& collisionManager)
{
    for (auto& enemy : m_savagePool)
    {
        if (!enemy.isActive()) continue;
        enemy.updateWithContext(deltaTime, playerPos, map);
        resolveEnemyCollision(enemy, map, collisionManager);
    }

    for (auto& enemy : m_chomperPool)
    {
        if (!enemy.isActive()) continue;
        enemy.updateWithContext(deltaTime, playerPos, map);

        // ChomperEnemy has its own applyCollisionCorrection to abort leaps on wall hit
        sf::FloatRect bounds = enemy.getBounds();
        auto worldResult = collisionManager.checkWorldCollisionDetailed(bounds, map);
        if (worldResult.collided)
            enemy.applyCollisionCorrection(collisionManager.resolveCollision(worldResult));

        // Object collision
        if (map)
        {

        }
    }
}

void EnemyManager::renderAll(sf::RenderTarget& target) const
{
    for (const auto& enemy : m_savagePool)
        if (enemy.isActive()) enemy.render(target);

    for (const auto& enemy : m_chomperPool)
        if (enemy.isActive()) enemy.render(target);
}

// Queries
int EnemyManager::getActiveSavageCount() const
{
    int count = 0;
    for (const auto& e : m_savagePool)
        if (e.isActive()) ++count;
    return count;
}

int EnemyManager::getActiveChomperCount() const
{
    int count = 0;
    for (const auto& e : m_chomperPool)
        if (e.isActive()) ++count;
    return count;
}

void EnemyManager::checkAttackHit(const sf::Vector2f& playerPos, const sf::Vector2f& targetPos, float damage, ItemType weaponType)
{
    // Melee: circle check around player. Ranged: distance along attack direction.
    const bool isMelee = (weaponType == ItemType::Knife || weaponType == ItemType::Axe);
    const float meleeRange = 80.f;
    const float gunRange = 800.f;
    const float hitRadius = 40.f;  // Enemy hit box radius for ray test

    auto tryHitSavage = [&](SavageEnemy& e) -> bool
        {
            if (!e.isActive()) return false;
            sf::Vector2f toEnemy = e.getPosition() - playerPos;
            float dist = std::sqrt(toEnemy.x * toEnemy.x + toEnemy.y * toEnemy.y);

            if (isMelee)
            {
                if (dist <= meleeRange)
                {
                    e.takeDamage(damage);
                    return true;
                }
            }
            else // Gun: check if enemy is near the line from player to target
            {
                if (dist > gunRange) return false;
                sf::Vector2f dir = targetPos - playerPos;
                float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
                if (len < 1.f) return false;
                dir /= len;
                // Perpendicular distance from enemy to ray
                sf::Vector2f perp(-dir.y, dir.x);
                float perpDist = std::abs(toEnemy.x * perp.x + toEnemy.y * perp.y);
                if (perpDist <= hitRadius)
                {
                    e.takeDamage(damage);
                    return true;
                }
            }
            return false;
        };

    for (auto& e : m_savagePool)  tryHitSavage(e);

    // Same logic for chompers
    for (auto& e : m_chomperPool)
    {
        if (!e.isActive()) continue;
        sf::Vector2f toEnemy = e.getPosition() - playerPos;
        float dist = std::sqrt(toEnemy.x * toEnemy.x + toEnemy.y * toEnemy.y);
        if (isMelee && dist <= meleeRange)
            e.takeDamage(damage);
        else if (!isMelee)
        {
            sf::Vector2f dir = targetPos - playerPos;
            float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
            if (len > 1.f)
            {
                dir /= len;
                sf::Vector2f perp(-dir.y, dir.x);
                float perpDist = std::abs(toEnemy.x * perp.x + toEnemy.y * perp.y);
                if (perpDist <= hitRadius && dist <= gunRange)
                    e.takeDamage(damage);
            }
        }
    }
}

// Collision
template<typename TEnemyType>
void EnemyManager::resolveEnemyCollision(TEnemyType& enemy,
    const Map* map,
    const CollisionManager& collisionManager)
{
    sf::FloatRect bounds = enemy.getBounds();

    auto worldResult = collisionManager.checkWorldCollisionDetailed(bounds, map);
    if (worldResult.collided)
    {
        sf::Vector2f correction = collisionManager.resolveCollision(worldResult);
        enemy.setPosition(enemy.getPosition() + correction);
    }
}