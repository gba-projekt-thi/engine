# Why the Sprite Wrapper?

The GBA only knows **screen coordinates** (240x160), but our game world is much larger. Butano's `bn::sprite_ptr` works in screen space, so if we just set its position to `(400, 300)` it would be off-screen and wrong.

Our `Sprite` class solves this by storing **world coordinates** (`world_x`, `world_y`) and converting them to screen coordinates each frame via the `Camera`:

```cpp
// Sprite keeps world position + a bn::sprite_ptr
Sprite frog_sprite(bn::sprite_items::frog.create_sprite(0, 0), start_x, start_y);

// sync() converts world -> screen using the camera
void sync(const Camera& camera) {
    _sprite.set_position(
        camera.to_screen_x(world_x),
        camera.to_screen_y(world_y)
    );
}
```

**Sprite Registry**

`SpriteRegistry::sync_all(camera)` calls `sync()` on every registered Sprite each frame.


