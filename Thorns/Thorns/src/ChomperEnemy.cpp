#include "ChomperEnemy.h"
#include "LineOfSight.h"
#include "MathUtilities.h"
#include "AssetPaths.h"
#include "Map.h"
#include <iostream>
#include <cmath>

ChomperEnemy::ChomperEnemy()
    : m_active(false)
    , m_aiState(AIState::Idle)
    , m_lastKnownPlayerPos(0.f, 0.f)
    , m_velocity(0.f, 0.f)
    , m_lostTimer(0.f)
    , m_leapDirection(0.f, 0.f)
    , m_leapTimer(0.f)
    , m_leapCooldownTimer(0.f)
{
}

bool ChomperEnemy::initialize(const std::string& atlasPath)
{
    if (!m_sprite.loadTexture(atlasPath, 64.f, 64.f,
        sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(64, 64))))
        return false;

    m_sprite.centerOrigin();
    m_active = true;
    return true;
}

// Basic update for test
void ChomperEnemy::update(sf::Time deltaTime)
{
    if (!m_active) return;
    updateMovement(deltaTime);
}

// Update with map and player
void ChomperEnemy::updateWithContext(sf::Time deltaTime,
    const sf::Vector2f& playerPos,
    const Map* map)
{
    if (!m_active) return;

    // Tick leap cooldown regardless of AI state
    if (m_leapCooldownTimer > 0.f)
        m_leapCooldownTimer -= deltaTime.asSeconds();

    updateAI(playerPos, map);
    updateMovement(deltaTime);
}

void ChomperEnemy::applyCollisionCorrection(const sf::Vector2f& correction)
{
    // If leaping into a wall, abort the leap
    if (m_aiState == AIState::Leap && (correction.x != 0.f || correction.y != 0.f))
    {
        m_aiState = AIState::Chase;
        m_velocity = sf::Vector2f(0.f, 0.f);
        m_leapCooldownTimer = LEAP_COOLDOWN;
    }

    m_sprite.move(correction);
}

// AI state machine
void ChomperEnemy::updateAI(const sf::Vector2f& playerPos, const Map* map)
{
    bool canSee = LineOfSight::hasLineOfSight(
        m_sprite.getPosition(), playerPos, map, LOS_RANGE);

    switch (m_aiState)
    {
    case AIState::Idle:
        m_velocity = sf::Vector2f(0.f, 0.f);
        if (canSee)
        {
            m_aiState = AIState::Chase;
            std::cout << "ChomperEnemy: Chase\n";
        }
        break;

    case AIState::Chase:
    {
        if (!canSee)
        {
            m_aiState = AIState::Lost;
            m_lostTimer = 0.f;
            std::cout << "ChomperEnemy: Lost\n";
            break;
        }

        m_lastKnownPlayerPos = playerPos;

        sf::Vector2f dir = playerPos - m_sprite.getPosition();
        float dist = MathUtils::magnitude(dir);

        // Check if close enough and cooldown expired to leap
        if (dist <= LEAP_RANGE && m_leapCooldownTimer <= 0.f)
        {
            beginLeap(playerPos);
            break;
        }

        if (dist > ARRIVAL_RADIUS)
        {
            m_velocity = MathUtils::normalize(dir) * CHASE_SPEED;
            faceDirection(dir);
        }
        else
        {
            m_velocity = sf::Vector2f(0.f, 0.f);
        }
        break;
    }

    case AIState::Leap:
    {
        // Leap travels along fixed direction until duration expires
        m_leapTimer += (1.f / 60.f);

        if (m_leapTimer >= LEAP_DURATION)
        {
            m_aiState = AIState::Chase;
            m_velocity = sf::Vector2f(0.f, 0.f);
            m_leapCooldownTimer = LEAP_COOLDOWN;
            std::cout << "ChomperEnemy: Leap finished, back to Chase\n";
        }
        break;
    }

    case AIState::Lost:
    {
        m_lostTimer += (1.f / 60.f);

        if (canSee)
        {
            m_aiState = AIState::Chase;
            break;
        }

        if (m_lostTimer >= LOST_DURATION)
        {
            m_aiState = AIState::Idle;
            m_velocity = sf::Vector2f(0.f, 0.f);
            std::cout << "ChomperEnemy: Idle\n";
            break;
        }

        sf::Vector2f dir = m_lastKnownPlayerPos - m_sprite.getPosition();
        float dist = MathUtils::magnitude(dir);

        if (dist > ARRIVAL_RADIUS)
        {
            m_velocity = MathUtils::normalize(dir) * (CHASE_SPEED * 0.5f);
            faceDirection(dir);
        }
        else
        {
            m_velocity = sf::Vector2f(0.f, 0.f);
        }
        break;
    }
    }
}

void ChomperEnemy::updateMovement(sf::Time deltaTime)
{
    if (m_velocity.x == 0.f && m_velocity.y == 0.f) return;
    m_sprite.move(m_velocity * deltaTime.asSeconds());
}

void ChomperEnemy::faceDirection(const sf::Vector2f& direction)
{
    // For facing right direction due to atlas direction
    float angle = MathUtils::vectorToAngleDegrees(direction) + SPRITE_NORTH_OFFSET;
    m_sprite.setRotation(sf::degrees(angle));
}

void ChomperEnemy::beginLeap(const sf::Vector2f& targetPos)
{
    sf::Vector2f dir = targetPos - m_sprite.getPosition();
    float dist = MathUtils::magnitude(dir);

    if (dist < 1.f) return;

    m_leapDirection = dir / dist;
    m_velocity = m_leapDirection * LEAP_SPEED;
    m_leapTimer = 0.f;
    m_aiState = AIState::Leap;

    faceDirection(dir);
    std::cout << "ChomperEnemy: Leap!\n";
}


void ChomperEnemy::render(sf::RenderTarget& target) const
{
    if (!m_active) return;
    m_sprite.render(target);
}

void ChomperEnemy::setPosition(const sf::Vector2f& pos)
{
    m_sprite.setPosition(pos);
}

sf::Vector2f ChomperEnemy::getPosition() const
{
    return m_sprite.getPosition();
}

sf::FloatRect ChomperEnemy::getBounds() const
{
    return m_sprite.getBounds();
}