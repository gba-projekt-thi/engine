# Class Overview

```
┌──────────────────────────────┐   ┌──────────────────────────────┐
│         StaticBody           │   │        CollisionShape        │
├──────────────────────────────┤   ├──────────────────────────────┤
│ pos : Position               │   │ shape_pos : Position         │
│ layers : uint16_t            │   │ mask : uint16_t              │
│ body_type : uint8_t          │   │                              │
│ sprite*                      │   │ get_collision()              │
│                              │   │                              │
│ enable | disable             │   │ shape_enable | shape_disable │
│ is_enabled()                 │   │ is_shape_enabled()           │
│                              │   │                              │
│                              │   │ on_enter() / on_exit()       │
│                              │   │ update()                     │
└──────┬───────────────┬───────┘   └───┬───────────┬──────────────┘
       ┆               ▲               ▲           ▲
       ┆               │               │           │
       ┆       ┌───────┴───────────────┴──┐        │
       ┆       │       PhysicsBody        │        │
       ┆       ├──────────────────────────┤        │
       ┆       │ block : uint16_t         │        │
       ┆       │ vel_x, vel_y, vel_max    │        │
       ┆       │                          │        │
       ┆       │ move(dx, dy)             │        │
       ┆       │ move_velocity()          │        │
       ┆       │ move_attachments()       │        │
       ┆       │                          │        │
       ┆       │ velocity                 │        │
       ┆       │   set | inc | dec        │        │
       ┆       │                          │        │
       ┆       │ probes                   │        │
       ┆       │   bottom|top|left|right  │        │
       ┆       └─────┬───────────┬────────┘        │
       ┆             ▲           ▲                 │
       ▼             │           │                 │
┌───────────┐  ┌─────┴─────┐ ┌───┴───────┐   ┌─────┴─────┐
│ «using»   │  │  Player   │ │   Enemy   │   │  Weapon   │
│  Door     │  ├───────────┤ ├───────────┤   ├───────────┤
└───────────┘  │ update()  │ │ update()  │   │ on_enter()│
               │ on_enter()│ │ move_     │   │ on_exit() │
               │ on_exit() │ │  attach.. │   └───────────┘
               └───────────┘ └───────────┘


┌──────────────────────────────┐   ┌──────────────────────────────┐
│        «Singleton»           │   │          «struct»            │
│      CollisionRegistry       │   │          Position            │
├──────────────────────────────┤   ├──────────────────────────────┤
│ load_map_rects()             │   │ x, y                         │
│ add_map_rect()               │   │ width, height                │
│                              │   │ offset_x, offset_y           │
│ check_rect()                 │   │                              │
│ check_point()                │   │ get_x(), get_y()             │
│                              │   │ move(x, y)                   │
│ update_all()                 │   └──────────────────────────────┘
└──────────────────────────────┘


┌────────────────────────┐   ┌──────────────────┐   ┌────────────────────────┐
│        «struct»        │   │    «struct»      │   │      «struct»          │
│    CollisionResult     │   │   CollisionHit   │   │       MapRect          │
├────────────────────────┤   ├──────────────────┤   ├────────────────────────┤
│ hits : CollisionHit[]  │   │ layers           │   │ x_min, y_min           │
│ count                  │   │ body             │   │ x_max, y_max, layers   │
│                        │   └──────────────────┘   └────────────────────────┘
│ combined_layers()      │
└────────────────────────┘
```

<br>

# Important Classes + Bitmasks

### StaticBody
- **Task:** Be discoverable
- **Provides:** position, layers bitmask
- **Bitmask:** `layers` — "What I am" — identifies this body to others
- **Usecases:** walls, pickups, trigger zones

### CollisionShape
- **Task:** Detect collisions
- **Provides:** `on_enter()`, `on_exit()`, `update()`, mask bitmask
- **Bitmask:** `mask` — "What I detect" — filters which bodies to detect
- **Usecases:** hitboxes, sensors, pickup range

### PhysicsBody
- **Task:** Move and interact
- **Provides:** `move()`, velocity, block bitmask
- **Bitmask:** `block` — "What blocks me" — stops `move()` in that direction
- **Usecases:** player, enemies, projectiles

<br>

# Unified Interface

`Position` and `enable` / `disable` work the same way across all classes —
`Sprite`, `StaticBody`, `CollisionShape`, `PhysicsBody`.

### Position
- Every class has a `pos` attribute

**`player.pos.move(x, y)` vs `player.move(x, y)`**
- `player.pos.move(x, y)` → teleports regardless of collision
- `player.move(x, y)` → moves only if collision allows

// `move_attachments(x, y)`
// `x == pos.x` and `y == pos.y`

### Enable / Disable
Every class has `enable()`, `disable()`, `is_enabled()`

- `Sprite`: shows / hides
- `StaticBody`: discoverable or not
- `CollisionShape`: callbacks fire or not
  - `disable()` → all `on_exit()` fire
  - `enable()` → all `on_enter()` fire
  - `get_collision()` still works when disabled

<br>

# Cast Collision Classes with `body_type`

This 8-Bit Integer determains the Class for Casting in `on_enter` / `on_exit`

**Problem**: Casting a *Enemy Class* in `on_enter` / `on_exit`

**Pre-Step**: Define a 8-Bit Number for *Enemy Class* 
- `0: Static Body`, `1: Physics Body`
- `2: Player Class`, `3: Enemy Class`, ... ...

**5 Checks to Victory**
```cpp
void on_enter(uint16_t hit_layers, StaticBody* body) override {
    // 1. cheap layer check
    if(hit_layers & ENEMY_LAYER) {
        // 2. check if body is not a nullptr (nullptr = map rect)
        if(body) {
            // 3. check if body_type matches the Enemy number
            if(body->body_type == 3) {
                // 4. safe to cast
                Enemy& enemy = static_cast<Enemy&>(*body);
                // 5. access the enemy's attributes
                enemy.hit(this.damage);
}   }   }   }
```

<br>

# `move_attachments()`

Move all dependent sprites and collision shapes that should synchronously move with this physics body.  
ex.: weapons, accessory sprites, additional collision shapes, ...

**Tipp:** Align attachment origins with the physics body using `Position.offset_x / offset_y`.  
Then `move_attachments(x, y)` just forwards `move(x, y)` — no per-attachment math.

<br>

**Stats for nerds — Call Order**
1. `update()` — logic + player position
2. `move_velocity()` — moves character with set velocity
3. `move_attachments()` — moves dependent shapes / sprites

`move_velocity()` runs automatically after `update()`, so attachments must move *after* that — which is why they get their own function.

<br>

# Array sizing

comming soon ...