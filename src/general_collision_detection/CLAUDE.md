# General Collision Detection - Butano GBA Project

## Butano Engine

This is a [Butano](https://github.com/GValiente/butano/tree/master) C++ GBA project. When anything about the engine API is unclear, **always** consult the [Butano documentation](https://gvaliente.github.io/butano/annotated.html) or the GitHub source before guessing. Butano uses fixed-point math (`bn::fixed`), its own smart pointers, and a specific asset pipeline — don't assume standard library equivalents exist.

## Folder Structure

```
general_collision_detection/
  graphics/        .bmp + .json files (Butano asset pipeline input)
  graphics_files/  source Aseprite (.aseprite/.ase) + Tiled (.tmx/.tsx) files
  include/         header files (.h)
  src/             source files (.cpp) + main.cpp
```

## Tilemap Pipeline

Tilemap collision data lives in `include/tilemap_data.h`. It is **generated** by `graphics_files/tilemap_8_8/convertcpp.py` from a Tiled `.tmx` file. The script merges solid tiles into axis-aligned rectangles (`MapRect`) and writes them as a constexpr array. Each `MapRect` has `x_min, y_min, x_max, y_max` (pixel coords) and a `layers` bitmask.

- Tilemap layer = `0x8000` (bit 15)
- `MapRect` is defined in `collision_registry.h`
- `DEFAULT_LAYERS` in `convertcpp.py` controls the layer value during generation

## Collision System

### Layer Bitmasks (16-bit)

| Field    | Lives in      | Purpose                                      |
|----------|---------------|----------------------------------------------|
| `layers` | `StaticBody`  | "What am I" — identifies this body            |
| `mask`   | `PhysicsBody` | "What I detect" — triggers `on_enter`/`on_exit` |
| `block`  | `PhysicsBody` | "What blocks me" — stops movement             |

Collision check: `(my_block & other.layers) != 0` means blocked. `(my_mask & other.layers) != 0` means overlap detected.

### StaticBody

Non-moving collision shape. Only has `layers`. Auto-registers with `CollisionRegistry` on construction, auto-unregisters on destruction. Optionally attach a `Sprite*` for visuals.

```cpp
// A door trigger — player detects it via mask, passes through it (not in player's block)
StaticBody door(door_x, door_y, 16, 25, Player::DOOR_LAYER);
door.sprite = &door_sprite;
```

### PhysicsBody

Extends `StaticBody` with velocity, movement, and collision resolution. Override `update()` for per-frame logic (input, AI, gravity). Override `on_enter()`/`on_exit()` for overlap callbacks. Movement resolves X and Y independently (enables wall-sliding).

```cpp
class Player : public PhysicsBody {
    static constexpr uint16_t LAYERS = 0x0001;
    static constexpr uint16_t MASK   = ENEMY_LAYER | DOOR_LAYER;
    static constexpr uint16_t BLOCK  = 0xFFFF & ~ENEMY_LAYER & ~DOOR_LAYER;

    Player(bn::fixed x, bn::fixed y, bn::fixed w, bn::fixed h)
        : PhysicsBody(x, y, w, h, LAYERS, MASK, BLOCK) {}

    void update() override { /* input, gravity, etc. */ }
    void on_enter(StaticBody& other) override { /* react to overlaps */ }
};
```

**Key functions:**

- `move(dx, dy)` — positional move with collision resolution
- `set_velocity()`, `inc_velocity()`, `dec_velocity()`, `apply_impulse()` — velocity control
- `probe_bottom()`, `probe_top()`, `probe_left()`, `probe_right()` — returns combined `layers` bitmask of anything 1px outside that edge (3 sample points per edge). Used for ground/wall checks.

### CollisionRegistry

Singleton. All `StaticBody`/`PhysicsBody` instances auto-register. `update_all()` calls `physics_update()` on every body with `needs_physics_update == true` (i.e., all `PhysicsBody` instances).

## Sprite System

### Sprite

Wraps a `bn::sprite_ptr` with world-space coordinates. Auto-registers with `SpriteRegistry` on construction. `sync()` converts world position to screen position via the Camera.

```cpp
Sprite frog_sprite(bn::sprite_items::frog.create_sprite(0, 0), world_x, world_y);
// automatically registered — no manual register call needed
```

Bodies reference sprites via a raw `Sprite*` pointer + optional offset:
```cpp
player.sprite = &frog_sprite;
player.sprite_offset_y = -7;  // visual sits 7px above collision center
```

`AnimatedSprite` is planned as a subclass of `Sprite` but **not yet implemented**.

### SpriteRegistry

Singleton. `sync_all(camera)` updates all registered sprites' screen positions each frame.

## Camera

Singleton. Follows a world-space target, clamped to map bounds (GBA screen: 240x160).

```cpp
Camera::instance().init(map_width * 8, map_height * 8);
Camera::instance().follow(player.x, player.y);
// use to_screen_x/y for world→screen, bg_x/y for background offset
```

## Main Loop Pattern

```cpp
while(true) {
    CollisionRegistry::instance().update_all();          // physics + collisions
    Camera::instance().follow(player.x, player.y);       // camera tracks player
    bg.set_position(Camera::instance().bg_x(), bg_y());  // scroll background
    SpriteRegistry::instance().sync_all(camera);         // world→screen sprites
    bn::core::update();                                  // flush to hardware
}
```
