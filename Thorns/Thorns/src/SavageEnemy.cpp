#include "SavageEnemy.h"
#include "LineOfSight.h"
#include "MathUtilities.h"
#include "AssetPaths.h"
#include <iostream>
#include <cmath>

SavageEnemy::SavageEnemy()
    : m_active(true)
    , m_aiState(AIState::Idle)
    , m_lastKnownPlayerPos(0.f, 0.f)
    , m_lostTimer(0.f)
    , m_velocity(0.f, 0.f)
{
}

bool SavageEnemy::initialize(const std::string& atlasPath)
{
    // First frame of the savage sprite sheet: 64x64 at origin
    if (!m_sprite.loadTexture(atlasPath, 64.f, 64.f,
        sf::IntRect(sf::Vector2i(0, 64), sf::Vector2i(64, 64))))
        return false;

    m_sprite.centerOrigin();
    return true;
}

void SavageEnemy::update(sf::Time deltaTime)
{
    if (!m_active) return;
    updateMovement(deltaTime);
}

void SavageEnemy::updateWithContext(sf::Time deltaTime,
    const sf::Vector2f& playerPos,
    const Map* map)
{
    if (!m_active) return;

    updateAI(playerPos, map);
    updateMovement(deltaTime);
}

void SavageEnemy::updateAI(const sf::Vector2f& playerPos, const Map* map)
{
    bool canSee = LineOfSight::hasLineOfSight(
        m_sprite.getPosition(), playerPos, map, LOS_RANGE);

    switch (m_aiState)
    {
    case AIState::Idle:
        if (canSee)
        {
            m_aiState = AIState::Chase;
            std::cout << "SavageEnemy: Chase\n";
        }
        // Idle: no velocity
        m_velocity = sf::Vector2f(0.f, 0.f);
        break;

    case AIState::Chase:
        m_lastKnownPlayerPos = playerPos;

        if (!canSee)
        {
            // Player broke LOS, start lost timer
            m_aiState = AIState::Lost;
            m_lostTimer = 0.f;
            std::cout << "SavageEnemy: Lost\n";
            break;
        }

        {
            // Move directly toward player
            sf::Vector2f dir = playerPos - m_sprite.getPosition();
            float dist = MathUtils::magnitude(dir);

            if (dist > ARRIVAL_RADIUS)
            {
                m_velocity = MathUtils::normalize(dir) * CHASE_SPEED;
                faceDirection(dir);
            }
            else
            {
                m_velocity = sf::Vector2f(0.f, 0.f);
            }
        }
        break;

    case AIState::Lost:
        m_lostTimer += (1.f / 60.f);

        if (canSee)
        {
            // Player came back into view
            m_aiState = AIState::Chase;
            break;
        }

        if (m_lostTimer >= LOST_DURATION)
        {
            m_aiState = AIState::Idle;
            m_velocity = sf::Vector2f(0.f, 0.f);
            std::cout << "SavageEnemy: Idle\n";
            break;
        }

        // Walk toward last known position during lost period
        {
            sf::Vector2f dir = m_lastKnownPlayerPos - m_sprite.getPosition();
            float dist = MathUtils::magnitude(dir);

            if (dist > ARRIVAL_RADIUS)
            {
                m_velocity = MathUtils::normalize(dir) * (CHASE_SPEED * 0.6f);
                faceDirection(dir);
            }
            else
            {
                m_velocity = sf::Vector2f(0.f, 0.f);
            }
        }
        break;
    }
}

void SavageEnemy::updateMovement(sf::Time deltaTime)
{
    if (m_velocity.x == 0.f && m_velocity.y == 0.f) return;
    m_sprite.move(m_velocity * deltaTime.asSeconds());
}

void SavageEnemy::faceDirection(const sf::Vector2f& direction)
{
    float angle = MathUtils::vectorToAngleDegrees(direction) + SPRITE_NORTH_OFFSET;
    m_sprite.setRotation(sf::degrees(angle));
}

void SavageEnemy::setPosition(const sf::Vector2f& pos)
{
    m_sprite.setPosition(pos);
}

sf::Vector2f SavageEnemy::getPosition() const
{
    return m_sprite.getPosition();
}

sf::FloatRect SavageEnemy::getBounds() const
{
    return m_sprite.getBounds();
}

void SavageEnemy::render(sf::RenderTarget& target) const
{
    if (!m_active) return;
    m_sprite.render(target);
}

void SavageEnemy::takeDamage(float amount)
{
    // Simple one-shot kill for now; expand with HealthComponent when ready
    m_active = false;
    std::cout << "SavageEnemy killed\n";
}

