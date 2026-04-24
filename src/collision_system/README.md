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


---

# Array sizing

---

# Static Body vs Collision Shape

---

# Off + new function

-> off only affects on_enter / on_exit
- does not affect probing functions
        - example: the jump still works while holding LB (disabeling the shape)

---

# Collision Registry

---

# Why `shape_` prefix in `CollisionShape`?

**Problem**: Multiinheritance + shadowing
If `CollisionShape` and `StaticBody` both have ex. a `pos` attribute, the `PhysicsBody` class then has two `pos` attributes due to Multiinheritance. 

---

# Unified Function signature

`enable()` `disable()` `is_enabled()`
For enabeling / disabling the class. The `is_enabled()` returns the state as bool.
- `Sprite`: Deceids if Butano shows the sprite or not.
- `CollisionShape`: Deceids if `on_enter` and `on_exit` trigger 
    - when calling `disabled()` all `on_exit()` will trigger.
    - when calling `enabled()` all `on_enter()` will trigger.
    - `get_collision` will still trigger when `enabled == off`.
- `StaticBody`: Deceids if Body is discoverable from all Physics interactions.  

`Position pos`
For position Info and movement functionality. 
<<class diagramm>>





  ┌────────────────────────────────────┬──────────────────────────────────────────────────────────────────────────────────────────────┐
  │                Item                │                                            Status                                            │
  ├────────────────────────────────────┼──────────────────────────────────────────────────────────────────────────────────────────────┤
  │ Make it work without tilemap       │ Done — map rects moved into CollisionRegistry via load_map_rects()                           │
  ├────────────────────────────────────┼──────────────────────────────────────────────────────────────────────────────────────────────┤
  │ CollisionShape class               │ Done — independent class with mask, on_enter/on_exit, update(), get_collision()              │
  ├────────────────────────────────────┼──────────────────────────────────────────────────────────────────────────────────────────────┤
  │ CollisionShape as parent of        │ Done — PhysicsBody inherits both StaticBody and CollisionShape                               │
  │ PhysicsBody                        │                                                                                              │
  ├────────────────────────────────────┼──────────────────────────────────────────────────────────────────────────────────────────────┤
  │ on_enter/on_exit with mask         │ Done — signature (uint16_t hit_layers, StaticBody* body), body is nullptr for map rects      │
  ├────────────────────────────────────┼──────────────────────────────────────────────────────────────────────────────────────────────┤
  │ Cast the original class in         │ Done — body_type uint8_t on StaticBody, Player/Enemy set their own constants, full casting   │
  │ on_enter                           │ example in Player                                                                            │
  ├────────────────────────────────────┼──────────────────────────────────────────────────────────────────────────────────────────────┤
  │ Static probe functions             │ Done — probes on PhysicsBody, return CollisionResult, use check_rect internally with 0xFFFF  │
  │                                    │ mask                                                                                         │
  ├────────────────────────────────────┼──────────────────────────────────────────────────────────────────────────────────────────────┤
  │ get_collision() single check       │ Done — on CollisionShape, works when disabled too                                            │
  ├────────────────────────────────────┼──────────────────────────────────────────────────────────────────────────────────────────────┤
  │ check_rect / check_point           │ Done — static functions on CollisionRegistry with optional skip parameter                    │
  ├────────────────────────────────────┼──────────────────────────────────────────────────────────────────────────────────────────────┤
  │ On/off for collision shapes        │ Done — shape_enabled flag, fires on_exit when disabled, update() always runs                 │
  ├────────────────────────────────────┼──────────────────────────────────────────────────────────────────────────────────────────────┤
  │ Shape offset from body             │ Done — shape_offset_x/y, set_shape(), auto-synced in move()                                  │
  ├────────────────────────────────────┼──────────────────────────────────────────────────────────────────────────────────────────────┤
  │ Combined layers helper             │ Done — CollisionResult::combined_layers() for quick bitmask checks                           │
  ├────────────────────────────────────┼──────────────────────────────────────────────────────────────────────────────────────────────┤
  │ Array renames                      │ Done — _static_bodies[], _collision_shapes[], _map_rects[]                                   │
  └────────────────────────────────────┴──────────────────────────────────────────────────────────────────────────────────────────────┘

  Still to do

  ┌──────────────────────────────────────┬────────────────────────────────────────────────────────────────────────────────────────┐
  │                 Item                 │                                         Notes                                          │
  ├──────────────────────────────────────┼────────────────────────────────────────────────────────────────────────────────────────┤
  │ Move collision boxes based on parent │ Offset system exists, but no parent pointer for child shapes (weapon following player) │
  ├──────────────────────────────────────┼────────────────────────────────────────────────────────────────────────────────────────┤
  │ Canvas layer for UI elements         │ Not started — UI sprites without registry entry                                        │
  ├──────────────────────────────────────┼────────────────────────────────────────────────────────────────────────────────────────┤
  │ On/off for sprites                   │ Not started — separate from collision shape enabled                                    │
  ├──────────────────────────────────────┼────────────────────────────────────────────────────────────────────────────────────────┤
  │ Get rid of make warning              │ The layers shadowing warning — could rename parameter in base class                    │
  ├──────────────────────────────────────┼────────────────────────────────────────────────────────────────────────────────────────┤
  │ Unload all sprites in SpriteRegistry │ Not started                                                                            │
  └──────────────────────────────────────┴────────────────────────────────────────────────────────────────────────────────────────┘
- Make a Debugger for shapes
