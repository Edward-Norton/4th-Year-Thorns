#include "EnemyManager.h"
#include "CollisionManager.h"
#include "Map.h"
#include <iostream>

EnemyManager::EnemyManager()
    : m_initialized(false)
{
}

bool EnemyManager::initialize(const std::string& savageAtlasPath,
    const std::string& chomperAtlasPath)
{
    
    for (auto& enemy : m_savagePool)
    {
        if (!enemy.initialize(savageAtlasPath))
        {
            std::cerr << "EnemyManager: Failed to initialize SavageEnemy pool slot\n";
            return false;
        }
        enemy.setActive(false);  
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

        
        sf::FloatRect bounds = enemy.getBounds();
        auto worldResult = collisionManager.checkWorldCollisionDetailed(bounds, map);
        if (worldResult.collided)
            enemy.applyCollisionCorrection(collisionManager.resolveCollision(worldResult));

        
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
    
    const bool isMelee = (weaponType == ItemType::Knife || weaponType == ItemType::Axe);
    const float meleeRange = 80.f;
    const float gunRange = 800.f;
    const float hitRadius = 40.f;  

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
            else 
            {
                if (dist > gunRange) return false;
                sf::Vector2f dir = targetPos - playerPos;
                float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
                if (len < 1.f) return false;
                dir /= len;
                
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