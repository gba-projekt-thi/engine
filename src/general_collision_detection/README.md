# Tilemap 

1. **Tilmap.md**: Shows how to create a tileset

---

# Why the Sprite Wrapper?

1. **World coordinates** 
2. **Collision system interface** 
3. **SpriteRegistry** 
4. **Base class for AnimatedSprite** (not implemented)

---

# Camera

- Follows a world-space target
- Clamps to map bounds
- Converts world -> screen coordinates
- Provides background scroll offsets

---

# Class Overview

```not_uml!
┌──────────────────────────────┐
│         StaticBody           │
├──────────────────────────────┤
│ x, y, width, height          │
│ layers : uint16_t            │
│ sprite*                      │
└──────────────┬───────────────┘
               ▲
┌──────────────┴───────────────┐
│         PhysicsBody          │
├──────────────────────────────┤
│ mask : uint16_t              │
│ block : uint16_t             │
│ move(dx, dy)                 │
│                              │
│ velocity                     │
│   set | inc | dec            │
│                              │
│ probes                       │
│   bottom | top | left | right│
│                              │
│ sprite_offset_x, _y          │
└───────┬──────────────┬───────┘
        ▲              ▲
┌───────┴───────┐ ┌────┴──────┐
│    Player     │ │   Enemy   │
├───────────────┤ └───────────┘
│ update()      │
│ on_enter()    │
│ on_exit()     │
└───────────────┘

┌───────────────────┐
│ CollisionRegistry │
├───────────────────┤
│ update_all()      │
└───────────────────┘
```

---

# Collision Layers

Every body uses three 16-bit bitmasks to control collision behavior. 

- **layers** — "What I am" — identifies this body to others
- **mask** — "What I detect" — triggers on_enter() / on_exit() callbacks
- **block** — "What blocks me" — stops move() in that direction