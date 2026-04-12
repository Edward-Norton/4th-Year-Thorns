#include "Player.h"
#include "InputController.h"
#include "MathUtilities.h"
#include "AssetPaths.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <iostream>

Player::Player()
    : m_currentState(PlayerState::Idle)
    , m_active(true)
    , m_inputController(nullptr)
    , m_mousePosition(0.f, 0.f)
    , m_velocity(0.f, 0.f)
    , m_targetRotation(sf::degrees(0.f))
    , m_currentRotation(sf::degrees(0.f))
    , m_hud(m_health, m_stamina, m_hunger, m_water)
{
}

bool Player::initialize(const std::string& texturePath)
{
    
    if (!m_sprite.loadTexture(texturePath, 34.f, 50.f,
        sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(34, 50))))
        return false;

    m_sprite.centerOrigin();
    m_sprite.setPosition(sf::Vector2f{ 960.f, 540.f });  

    
    if (!m_cursor.initialize(8.f))
        return false;

    
    
    m_health.initialize(100.0f);

    
    m_stamina.initialize(100.f, 0.f);

    
    m_hunger.initialize(100.f, 0.5f);

    
    m_water.initialize(100.f, 0.8f);

    
    if (!m_hud.initialize(Assets::Fonts::JERSEY_20))
    {
        std::cerr << "Player: HUD font failed to load, health bar will not render\n";
    }

    
    m_inventory.initialize();

    
    m_inventory.setOnItemUsed([this](ItemType type) { onItemUsed(type); });

    m_cursor.setColor(sf::Color::Yellow);

    return true;
}

void Player::update(sf::Time deltaTime)
{
    if (!m_active) return;

    
    updateState();

    
    updateCursor();

    
    updateRotation();

    
    m_health.update(deltaTime);
    if (m_currentState != PlayerState::InventoryOpen)
    {
        m_hunger.update(deltaTime);
        m_water.update(deltaTime);
    }

    
    updateMovement(deltaTime);
}

void Player::updateWithInput(sf::Time deltaTime, const InputController& input, const sf::Vector2f& mousePosition)
{
    if (!m_active) return;

    
    m_inputController = &input;
    m_mousePosition = mousePosition;

    if (input.wasJustPressed(InputAction::Inventory))
    {
        m_inventory.toggle();

        
        if (m_inventory.isVisible())
        {
            changeState(PlayerState::InventoryOpen);
        }
        else
        {
            
            changeState(PlayerState::Idle);
        }
    }

    
    if (m_inventory.isVisible())
    {
        m_inventory.updateSlotInteraction(input);
    }

    
    update(deltaTime);
}

void Player::render(sf::RenderTarget& target) const
{
    if (!m_active) return;
    m_sprite.render(target);
}

void Player::renderHUD(sf::RenderTarget& target) const
{
    m_hud.render(target);
}

void Player::renderCursor(sf::RenderTarget& target) const
{
    m_cursor.render(target);
}

void Player::renderInventory(sf::RenderTarget& target) const
{
    m_inventory.render(target);
}

void Player::equipWeapon(ItemType type)
{
    m_equippedWeapon = type;
    std::cout << "Weapon equipped: " << static_cast<int>(type) << "\n";
}

float Player::attack(const sf::Vector2f& targetWorldPos)
{
    if (m_equippedWeapon == ItemType::COUNT)
    {
        std::cout << "No weapon equipped\n";
        return 0.f;
    }

    static const float damageTable[] = { 0.f, 0.f, 0.f, 0.f, 0.f, 25.f, 50.f, 75.f };
    
    int idx = static_cast<int>(m_equippedWeapon);
    float dmg = (idx >= 0 && idx < 8) ? damageTable[idx] : 0.f;

    std::cout << "Attack with weapon " << idx << " for " << dmg << " damage\n";
    return dmg;
}

void Player::updateState()
{
    if (!m_inputController) return;

    
    if (m_currentState == PlayerState::InventoryOpen)
    {
        return; 
    }

    
    sf::Vector2f moveInput = calculateMovementInput();
    bool isMoving = (moveInput.x != 0.f || moveInput.y != 0.f);

    
    bool isSprinting = m_inputController->isPressed(InputAction::Sprint) && !m_stamina.isEmpty();

    
    PlayerState newState = m_currentState;

    if (!isMoving)
    {
        newState = PlayerState::Idle;
    }
    else if (isSprinting)
    {
        newState = PlayerState::Sprint;
    }
    else
    {
        newState = PlayerState::Walk;
    }

    
    if (newState != m_currentState)
    {
        changeState(newState);
    }
}

void Player::changeState(PlayerState newState)
{
    
    switch (m_currentState)
    {
    case PlayerState::Idle:
        break;
    case PlayerState::Walk:
        break;
    case PlayerState::Sprint:
        break;
    case PlayerState::InventoryOpen:
        break;
    }

    
    m_currentState = newState;

    
    switch (m_currentState)
    {
    case PlayerState::Idle:
        std::cout << "State: IDLE\n";
        break;
    case PlayerState::Walk:
        std::cout << "State: WALK\n";
        break;
    case PlayerState::Sprint:
        std::cout << "State: SPRINT\n";
        break;
    case PlayerState::InventoryOpen:
        std::cout << "State: INVENTORY OPEN\n";
    }
}

void Player::updateMovement(sf::Time deltaTime)
{
    if (!m_inputController) return;

    if (m_currentState == PlayerState::InventoryOpen)
    {
        
        m_velocity = sf::Vector2f(0.f, 0.f);
        return;
    }

    float dt = deltaTime.asSeconds();

    
    sf::Vector2f moveInput = calculateMovementInput();
    float inputMagnitude = MathUtils::magnitude(moveInput);

    if (inputMagnitude > 0.f)
    {
        
        sf::Vector2f direction = MathUtils::normalize(moveInput);
        float targetSpeed = getCurrentSpeed();

        
        m_velocity = direction * targetSpeed;
    }
    else
    {
        
        float currentSpeed = MathUtils::magnitude(m_velocity);

        if (currentSpeed > 0.f)
        {
            sf::Vector2f direction = MathUtils::normalize(m_velocity);
            float decel = DECELERATION * dt;
            currentSpeed = std::max(0.f, currentSpeed - decel);
            m_velocity = direction * currentSpeed;
        }
    }

    if (m_currentState == PlayerState::Sprint)
    {
        m_stamina.decrease(STAMINA_SPRINT_DRAIN * dt);
    }
    else
    {
        m_stamina.increase(STAMINA_REGEN * dt);
    }

    
    sf::Vector2f movement = m_velocity * dt;
    m_sprite.move(movement);
}

void Player::updateRotation()
{
    if (m_currentState == PlayerState::InventoryOpen)
    {
        
        return;
    }

    
    sf::Vector2f playerPos = m_sprite.getPosition();
    sf::Vector2f direction = m_mousePosition - playerPos;

    
    float angleDeg = MathUtils::vectorToAngleDegrees(direction);

    
    m_targetRotation = sf::degrees(angleDeg);
    m_currentRotation = m_targetRotation;
    m_sprite.setRotation(m_currentRotation);
}

void Player::updateCursor()
{
    m_cursor.update(m_inputController->getMousePosition());
}

sf::Vector2f Player::calculateMovementInput() const
{
    if (!m_inputController) return sf::Vector2f(0.f, 0.f);

    sf::Vector2f input(0.f, 0.f);

    if (m_inputController->isPressed(InputAction::MoveUp))    input.y -= 1.f;
    if (m_inputController->isPressed(InputAction::MoveDown))  input.y += 1.f;
    if (m_inputController->isPressed(InputAction::MoveLeft))  input.x -= 1.f;
    if (m_inputController->isPressed(InputAction::MoveRight)) input.x += 1.f;

    return input;
}

float Player::getCurrentSpeed() const
{
    switch (m_currentState)
    {
    case PlayerState::Idle:
        return 0.f;
    case PlayerState::InventoryOpen:
        return 0.f;
    case PlayerState::Walk:
        return WALK_SPEED;
    case PlayerState::Sprint:
        return SPRINT_SPEED;
    default:
        return 0.f;
    }
}

void Player::onItemUsed(ItemType type)
{
    switch (type)
    {
    case ItemType::Food:
        m_hunger.increase(30.f);
        std::cout << "Ate food. Hunger: " << m_hunger.getValue() << "\n";
        break;
    case ItemType::Water:
        m_water.increase(40.f);
        std::cout << "Drank water. Water: " << m_water.getValue() << "\n";
        break;
    case ItemType::FirstAid:
        m_health.heal(100.f);
        std::cout << "Used First Aid. HP: " << m_health.getCurrentHealth() << "\n";
        break;
    case ItemType::Bandage:
        m_health.heal(35.f);
        std::cout << "Used Bandage. HP: " << m_health.getCurrentHealth() << "\n";
        break;
    case ItemType::Knife:
        equipWeapon(type);
    case ItemType::Axe:
        equipWeapon(type);
    case ItemType::Gun:
        equipWeapon(type);
        std::cout << "Equipped weapon (type " << static_cast<int>(type) << ")\n";
        break;
    default:
        break;
    }
}

void Player::setPosition(const sf::Vector2f& pos)
{
    m_sprite.setPosition(pos);
}

sf::Vector2f Player::getPosition() const
{
    return m_sprite.getPosition();
}

sf::FloatRect Player::getBounds() const
{
    return m_sprite.getBounds();
}

bool Player::collectItem(const ItemTypeData& data, const sf::Texture& atlas)
{
    
    return m_inventory.addItem(data.name, atlas, data.atlasRect, data.itemType);
}
