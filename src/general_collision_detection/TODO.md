- move `sprite` from StaticBody to Physics body
- is the camerapostion with `Position`-Struct?

---

<br>

# Debug system

- Namespace: `debug::`
- uses `get_x()` and `get_y()` functions from `Position` Struct

**Main Functions**:
```cpp
debug::register_draw_position(Position, color);
debug::register_draw_positions(Positions*, nr_pos, color);

// Inside Loop
debug::Registry.update_all(Camera*);
```

**Optional**
```cpp
debug::register_draw_sprite(Sprite, color);
debug::register_draw_sprites(Sprites*, nr_sprites, color);
debug::register_draw_static_body(StaticBody, color);
debug::register_draw_static_bodies(StaicBodies*, nr_bodies, color);
debug::register_draw_collision_shape(CollisionShape, color);
debug::register_draw_collision_shapes(ColisionShapes, nr_shapes, color);
```

---

<br>

# Unload System

- Reset Sprites by = 0 every array
