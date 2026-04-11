#pragma once

#ifndef ENEMY_MANAGER_H
#define ENEMY_MANAGER_H

#include <array>
#include <SFML/Graphics.hpp>
#include <SFML/System/Time.hpp>
#include "SavageEnemy.h"
#include "ChomperEnemy.h"
#include "ItemType.h"

class Map;
class CollisionManager;

/// <summary>
/// EnemyManager is just a basic pool pattern
/// 
/// Collision is resolved here
/// </summary>
class EnemyManager
{
public:
    // Pool capacities
    static constexpr int MAX_SAVAGE = 8;
    static constexpr int MAX_CHOMPER = 8;

    EnemyManager();
    ~EnemyManager() = default;

    // ========== Initialization ==========
    bool initialize(const std::string& savageAtlasPath,
        const std::string& chomperAtlasPath);

    // ========== Spawning ==========
    // Returns pointer to the activated slot, or nullptr when pool is full.
    SavageEnemy* spawnSavage(const sf::Vector2f& worldPos);
    ChomperEnemy* spawnChomper(const sf::Vector2f& worldPos);

    // Deactivate a specific enemy
    void despawnSavage(SavageEnemy* enemy);
    void despawnChomper(ChomperEnemy* enemy);

    // Deactivate all enemies
    void despawnAll();

    // ========== Per-frame ==========
    // Runs AI, movement, then collision resolution for every active enemy
    void updateAll(sf::Time deltaTime,
        const sf::Vector2f& playerPos,
        const Map* map,
        const CollisionManager& collisionManager);

    void renderAll(sf::RenderTarget& target) const;

    // ========== Queries ==========
    int getActiveSavageCount()  const;
    int getActiveChomperCount() const;

    // Check if a player attack hits any active enemy.
    // meleeRange is used for Knife/Axe, Gun uses a ray check.
    void checkAttackHit(const sf::Vector2f& playerPos,
        const sf::Vector2f& targetPos,
        float damage,
        ItemType weaponType);

private:
    // PN: had to use interface tag for this reason
    template<typename TEnemyType>
    void resolveEnemyCollision(TEnemyType& enemy,
        const Map* map,
        const CollisionManager& collisionManager);

    std::array<SavageEnemy, MAX_SAVAGE>  m_savagePool;
    std::array<ChomperEnemy, MAX_CHOMPER> m_chomperPool;

    bool m_initialized = false;
};

#endif