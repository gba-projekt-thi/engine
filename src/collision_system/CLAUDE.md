# Overview

This is a [Butano](https://github.com/GValiente/butano/tree/master) C++ GBA project. When anything about the engine API is unclear, **always** consult the [Butano documentation](https://gvaliente.github.io/butano/annotated.html) or the GitHub source before guessing.

The shared engine code (collision system, sprite system, later scene manager) lives in `../../core/` — headers in `./include/`, sources in `./src/`, API write-ups in `./documentation/`:
- `CollisionSystem.md` — StaticBody, CollisionShape, PhysicsBody, CollisionRegistry
- `Sprites.md` — Sprite wrapper, SpriteRegistry, Camera

**Naming conventions:**
- `MASK_*` — 16-bit layer bitmasks (e.g. `MASK_ENEMY`, `MASK_TILEMAP`)
- `TYPE_*` — 8-bit `body_type` IDs, used to safely `static_cast` in `on_enter` / `on_exit` (e.g. `TYPE_ENEMY = 3`)

# Tilemap

Tilemap collision data lives in `include/tilemap_data.h` as a constexpr array of `MapRect`s. Tilemap layer = `0x8000` (bit 15), exposed as `MASK_TILEMAP`.

# Collision System

## Layer Bitmasks (16-bit)

Three fields drive everything: `layers` ("what I am", on `StaticBody`), `mask` ("what I detect", on `CollisionShape`), `block` ("what stops me", on `PhysicsBody`).

## Important Classes

All three auto-register with `CollisionRegistry` on construction, auto-unregister on destruction — never call register manually.

- **`StaticBody`** — discoverable only. Has `layers`. Use for walls, pickups, triggers.
- **`CollisionShape`** — detects overlaps via `mask`, fires `on_enter()` / `on_exit()`. Use for hitboxes, sensors, pickup range.
- **`PhysicsBody`** — moves via `move()` / velocity, blocks via `block`. Override `update()` for per-frame logic. Use for player, enemies, projectiles.

## Unified Interface

`pos`, `enable()`, `disable()`, `is_enabled()` work identically across `Sprite`, `StaticBody`, `CollisionShape`, `PhysicsBody`.

**Gotchas:**
- `CollisionShape` prefixes these as `shape_pos`, `shape_enable`, ... — `PhysicsBody` inherits from both and the unprefixed names would collide.
- `body.pos.move(x, y)` teleports; `body.move(x, y)` resolves collisions.
- Disabling a `CollisionShape` fires `on_exit()` for everything it was overlapping; enabling fires `on_enter()` for everything it currently overlaps.

## body_type Casting

Safe downcast in `on_enter()` / `on_exit()`: cheap layer check → null check (map rects pass `nullptr`) → `body_type` match → cast.

```cpp
void on_enter(uint16_t hit_layers, StaticBody* body) override {
    if ((hit_layers & MASK_ENEMY) && body && body->body_type == TYPE_ENEMY) {
        static_cast<Enemy&>(*body).hit(damage);
    }
}
```

## Probes

`probe_bottom()`, `probe_top()`, `probe_left()`, `probe_right()` (on `PhysicsBody`) return a `CollisionResult` for a region 1px outside the given edge. Use `combined_layers()` to AND against a mask.

```cpp
bool grounded = probe_bottom(MASK_TILEMAP).any();
```

See `CollisionSystem.md` and `CollisionRegistry::check_rect` for the underlying query.

## move_attachments()

Override on `PhysicsBody` to move dependent sprites, shapes, or weapons that should track this body. Runs *after* `move_velocity()` (not before) — that's why it's its own hook. Align attachment origins via `Position.offset_x/y` so you can just forward `move(dx, dy)` without per-attachment math.

## Registry

Singleton. `StaticBody`, `CollisionShape`, `PhysicsBody`, and `MapRect` all auto-register on construction; `update_all()` runs every frame and drives collision callbacks. See docs for `check_rect()` / `check_point()` one-shot queries.

**Caps:** 64 static bodies · 16 physics bodies · 32 collision shapes · 32 map rects · 8 hits per `CollisionResult` (so a `CollisionShape` sees at most 8 overlaps at once).

# Sprite System

Butano sprites are **screen-space**; game logic (movement, collision, AI) stays in **world-space**. The `Sprite` wrapper holds a world-space position; `SpriteRegistry::sync_all(camera)` converts world → screen each frame.

## Sprite

Wraps a `bn::sprite_ptr` with a world-space `pos`. Auto-registers with `SpriteRegistry` on construction. Bodies reference a sprite via raw `Sprite*` + optional `sprite_offset_x/y` for visual-vs-hitbox alignment.

## SpriteRegistry

Singleton. `sync_all(camera)` pushes every registered sprite's screen position each frame. Cap: 64 sprites.

## Camera

Singleton. `follow(x, y)` tracks a world-space target, clamped to the map bounds passed into `init(world_w, world_h)`. Exposes `to_screen_x/y` for sprites and `bg_x/y` for background scrolling.

## Canvas Layer (UI)

Screen-locked overlays — health bars, menus, score — skip the wrapper entirely. Use the Butano sprite API directly; no registry, no camera, no world coordinates.
