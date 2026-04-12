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

class EnemyManager
{
public:
    
    static constexpr int MAX_SAVAGE = 8;
    static constexpr int MAX_CHOMPER = 8;

    EnemyManager();
    ~EnemyManager() = default;

    
    bool initialize(const std::string& savageAtlasPath,
        const std::string& chomperAtlasPath);

    
    
    SavageEnemy* spawnSavage(const sf::Vector2f& worldPos);
    ChomperEnemy* spawnChomper(const sf::Vector2f& worldPos);

    
    void despawnSavage(SavageEnemy* enemy);
    void despawnChomper(ChomperEnemy* enemy);

    
    void despawnAll();

    
    
    void updateAll(sf::Time deltaTime,
        const sf::Vector2f& playerPos,
        const Map* map,
        const CollisionManager& collisionManager);

    void renderAll(sf::RenderTarget& target) const;

    
    int getActiveSavageCount()  const;
    int getActiveChomperCount() const;

    
    
    void checkAttackHit(const sf::Vector2f& playerPos,
        const sf::Vector2f& targetPos,
        float damage,
        ItemType weaponType);

private:
    
    template<typename TEnemyType>
    void resolveEnemyCollision(TEnemyType& enemy,
        const Map* map,
        const CollisionManager& collisionManager);

    std::array<SavageEnemy, MAX_SAVAGE>  m_savagePool;
    std::array<ChomperEnemy, MAX_CHOMPER> m_chomperPool;

    bool m_initialized = false;
};

#endif