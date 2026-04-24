# Class Overview

```
┌──────────────────────────────┐
│            Sprite            │
├──────────────────────────────┤
│ pos : Position               │
│                              │
│ enable | disable             │
│ is_enabled()                 │
│                              │
│ sync(camera)                 │
│ sprite() : bn::sprite_ptr&   │
└──────────────────────────────┘


┌──────────────────────────────┐   ┌──────────────────────────────┐
│        «Singleton»           │   │        «Singleton»           │
│        SpriteRegistry        │   │            Camera            │
├──────────────────────────────┤   ├──────────────────────────────┤
│ register_sprite()            │   │ init(world_w, world_h)       │
│ unregister_sprite()          │   │ follow(x, y)                 │
│                              │   │                              │
│ sync_all(camera)             │   │ to_screen_x() / y()          │
│                              │   │ bg_x() / bg_y()              │
└──────────────────────────────┘   └──────────────────────────────┘


┌────────────────────────┐
│        «struct»        │
│        Position        │
├────────────────────────┤
│ x, y                   │
│ width, height          │
│ offset_x, offset_y     │
│                        │
│ get_x(), get_y()       │
│ move(x, y)             │
└────────────────────────┘
```

<br>

<br>

# Why the Sprite Wrapper?

### World vs Screen Coordinates

Butano's system uses **screen space** which shifts whenever the camera moves. So game logic (movement, collision, AI) stays in **world coordinates**, and the per-frame `sync(camera)` step converts to screen space — handled by `SpriteRegistry`, see below.

`Sprite` uses the `Position` struct like the collision system — in world coordinates.

Writing a `Player` class from `PhysicsBody` will move the `Sprite` on every movement. So a Sprite Wrapper provides that interface.

<br>

<br>

# Camera

`Camera` (singleton) provides the offset for local coordinates.

- **`init(world_w, world_h)`** — ensures the view stays inside the world area
- **`follow()`** — follows an object (Player, "Kamerafahrt"-Module)
- **`to_screen()`** — offset for local coordinates
- **`bg()`** — offset for the background scrolling

<br>

<br>

# SpriteRegistry

`SpriteRegistry` (singleton) keeps track of all sprites. It provides a function to sync every sprite to the camera each frame.

**Array Size**:
- `MAX_SPRITES = 64`

<br>

<br>

# Canvas Layer

A **canvas layer** is a screen-locked overlay — it stays fixed to the display and ignores the camera. Perfect for UI elements like health bars, menus, debug text, or score displays.

The `Sprite` wrapper + `Camera` + `SpriteRegistry` are only needed for world-space objects. For screen-locked stuff, just use the butano system directly — no registry, no sync, no world coordinates.

<br>

<br>

# Later

### AnimatedSprite

comming soon ...

<br>

---

### Palette

comming maybe ... 