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
       ┆       │ probes(mask = 0xFFFF)    │        │
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
│ any()                  │
│ combined_layers()      │
└────────────────────────┘
```

<br>

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

<br>

# Unified Interface

`Position` and `enable` / `disable` work the same way across all classes —
`Sprite`, `StaticBody`, `CollisionShape`, `PhysicsBody`.

<br>

---

### Position
- Every class has a `pos` attribute

**`player.pos.move(x, y)` vs `player.move(x, y)`**
- `player.pos.move(x, y)` → teleports regardless of collision
- `player.move(x, y)` → moves only if collision allows

<br>

---

### Enable / Disable
Every class has `enable()`, `disable()`, `is_enabled()`

- `Sprite`: shows / hides
- `StaticBody`: discoverable or not
- `CollisionShape`: callbacks fire or not
  - `disable()` → all `on_exit()` fire
  - `enable()` → all `on_enter()` fire
  - `get_collision()` still works when disabled

<br>

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

<br>

# `move_attachments()`

Move all dependent sprites and collision shapes that should synchronously move with this physics body.  
ex.: weapons, accessory sprites, additional collision shapes, ...

**Tipp:** Align attachment origins with the physics body using `Position.offset_x / offset_y`.  
Then `move_attachments(x, y)` just forwards `move(x, y)` — no per-attachment math.

<br>

<br>

**Stats for nerds — Call Order**
1. `update()` — logic + player position
2. `move_velocity()` — moves character with set velocity
3. `move_attachments()` — moves dependent shapes / sprites

`move_velocity()` runs automatically after `update()`, so attachments must move *after* that — which is why it gets it's own function.

<br>

<br>

# Registry

The `CollisionRegistry` is the central bookkeeping system (Singleton) for all collision data. Every `StaticBody`, `PhysicsBody`, and `CollisionShape` registers itself here on creation.

<br>

---

### Array of Registered Elements
The Registry holds for `StaticBody` `CollisionShape` `PhysicsBody` `MapRect` an Array and `register_..()` / `unregister_..()` functions. 

**Array Sizes**:
- `MAX_STATIC_BODIES = 64`
- `MAX_PHYSICS_BODIES = 16`
- `MAX_COLLISION_SHAPES = 32`
- `MAX_MAP_RECTS = 32`

<br>

---

### MapRect
A lightweight collision rectangle (like Physics Body) generated from the tilemap — no class, just coordinates and a `layers` bitmask. See Struct `MapRect`. 

<br>

---

### Collision Result
An Array `hits` of Collision Hits with `count`. 

**`CollisionHit`**: Contains `Layer` & `StaticBody` (like `on_enter()`)

**`any()`**: `true` if the probe hit anything (after mask filtering). Use when you only care *whether* something was hit.

**`combined_layers()`**: Returns a single 16 Bit value to compare with a mask.

```cpp
// Probes take a mask — filtering happens inside check_rect, not after.
bool grounded = player.probe_bottom(MASK_TILEMAP).any();
```

```cpp
// Damage all Enemies on a Pixel
CollisionResult res = CollisionRegistry::check_point(x, y, MASK_ENEMY);
for(int i = 0; i < res.count; i++){
    StaticBody* body = res.hits[i].body;
    if(body && body->body_type == TYPE_ENEMY){
        Enemy& enemy = static_cast<Enemy&>(*body);
        enemy.dec_health(20);
    }
}
```

**Array Size**: `MAX_HITS = 8`: Maximum number of Physics Bodies every Collision can see at once

<br>

---

### `check_rect()` / `check_point()`
One-shot collision queries against static bodies and map rects. Used under the hood by the classes.

**Example Custom Treasure Probe**

```cpp
bool probe_treasure_behind_wall() {
    auto res = CollisionRegistry::check_rect(
        pos.x + offset_x, pos.y + offset_y,
        width, height, TREASURE_MASK);
    return res.count > 0;
}
```

<br>

<br>

# Collision Config

`collision_config.h` collects every `MASK_*` and `TYPE_*` value in one place. It's a proposed pattern, not a requirement.

**`MASK_*`**: 16-bit layer bitmasks — one bit per layer, used for `layers`, `mask`, and `block`.
**ex.:** `constexpr uint16_t MASK_PLAYER = 0x0001;`

**`TYPE_*`**: 8-bit body-type IDs for any `StaticBody` subclass that needs to be cast in `on_enter()` / `on_exit()`.
**ex.:** `constexpr uint8_t TYPE_PLAYER = 2;`

<br>

<br>
