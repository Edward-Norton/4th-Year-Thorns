### SpriteComponent

Wraps SFML sprite with texture loading and transformation utilities.
**Key Features:**
- Texture atlas support (load specific regions)
- Scale-based sizing system
- Rotation and origin manipulation
- Bounds queries for collision detection
```cpp
SpriteComponent sprite;
sprite.loadTexture("player.png", 34.f, 50.f);
sprite.centerOrigin();
sprite.setPosition({960.f, 540.f});
```
### ButtonComponent

Interactive button with state-based visuals and callback execution.
**States:**
- Normal: Default gray appearance
- Hovered: Lighter gray, mouse over
- Pressed: Darker, mouse clicking
- Selected: Keyboard/gamepad selection (yellow highlight)
- Disabled: Grayed out, non-interactive

**Click Detection:**
Detects click when mouse is released over button (not pressed).

**Callback System:**
Stores and executes lambda functions when clicked or activated.

```cpp
button.setCallback([this]() { onStartGame(); });
button.activate();  // Manually trigger callback
```

### CursorComponent

Custom circular cursor with outline, replaces OS cursor during gameplay.
**Features:**
- Circular with white outline
- Centered on mouse position
- Toggleable visibility
- Color customization
```cpp
cursor.initialize(8.f);  // 8px radius
cursor.setColor(sf::Color::Yellow);
cursor.setVisible(false);  // Hide during menu
```