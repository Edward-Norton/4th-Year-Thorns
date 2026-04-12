#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "SpriteComponent.h"
#include "CursorComponent.h"
#include "IGameEntity.h"
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>
#include "Inventory.h"
#include "HealthComponent.h"
#include "HudComponent.h"
#include "ItemType.h"

class InputController;

enum class PlayerState
{
    Idle,   
    Walk,   
    Sprint,  
    InventoryOpen 
};

class Player : public IGameEntity
{
public:
    Player();
    ~Player() = default;

    
    bool initialize(const std::string& texturePath);

    
    void update(sf::Time deltaTime) override;

    
    
    void updateWithInput(sf::Time deltaTime, const InputController& input, const sf::Vector2f& mousePosition);

    
    void render(sf::RenderTarget& target) const override;
    void renderHUD(sf::RenderTarget& target) const;
    void renderCursor(sf::RenderTarget& target) const;
    void renderInventory(sf::RenderTarget& target) const;

    
    void setPosition(const sf::Vector2f& pos) override;
    sf::Vector2f getPosition() const override;

    
    sf::FloatRect getBounds() const override;

    
    bool isActive() const override { return m_active; }
    void setActive(bool active) override { m_active = active; }

    
    bool isValid() const { return m_sprite.isValid(); }
    PlayerState getCurrentState() const { return m_currentState; }

    
    CursorComponent& getCursor() { return m_cursor; }
    const CursorComponent& getCursor() const { return m_cursor; }

    
    Inventory& getInventory() { return m_inventory; }
    const Inventory& getInventory() const { return m_inventory; }
    bool collectItem(const ItemTypeData& data, const sf::Texture& atlas);

    
    HealthComponent& getHealth() { return m_health; }
    const HealthComponent& getHealth() const { return m_health; }

    StatComponent& getStamina() { return m_stamina; }
    const StatComponent& getStamina() const { return m_stamina; }

    StatComponent& getHunger() { return m_hunger; }
    const StatComponent& getHunger() const { return m_hunger; }

    StatComponent& getWater() { return m_water; }
    const StatComponent& getWater() const { return m_water; }

    
    void equipWeapon(ItemType type);
    ItemType getEquippedWeapon() const { return m_equippedWeapon; }
    float attack(const sf::Vector2f& targetWorldPos);

private:
    
    void updateState();
    void changeState(PlayerState newState);

    
    void updateMovement(sf::Time deltaTime);
    void updateRotation();
    void updateCursor();
    sf::Vector2f calculateMovementInput() const;
    float getCurrentSpeed() const;

    
    void onItemUsed(ItemType type);

    
    SpriteComponent m_sprite;
    CursorComponent m_cursor;
    Inventory m_inventory;
    
    HealthComponent m_health;
    StatComponent   m_stamina;
    StatComponent   m_hunger;
    StatComponent   m_water;
    HUDComponent m_hud;   

    ItemType m_equippedWeapon = ItemType::COUNT;

    
    PlayerState m_currentState;
    bool m_active;

    
    const InputController* m_inputController;  
    sf::Vector2f m_mousePosition;

    
    sf::Vector2f m_velocity;
    sf::Angle m_targetRotation;
    sf::Angle m_currentRotation;

    
    static constexpr float WALK_SPEED = 150.0f;
    static constexpr float SPRINT_SPEED = 250.0f;
    static constexpr float ROTATION_SPEED = 360.0f;  
    static constexpr float ACCELERATION = 1200.0f;   
    static constexpr float DECELERATION = 1800.0f;   
    static constexpr float FRICTION = 0.85f;         

    
    static constexpr float STAMINA_SPRINT_DRAIN = 15.0f;
    static constexpr float STAMINA_REGEN = 5.0f;
};

#endif