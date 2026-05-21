# Class Overview

```
┌──────────────────────────────────────────────┐
│              «namespace»                     │
│               pos_debug                      │
├──────────────────────────────────────────────┤
│  register_draw_position(pos, color)          │
│  register_draw_positions(arr, count, color)  │
│                                              │
│  register_draw_rect(x_min,y_min,             │
│                     x_max,y_max, color)      │
│  register_draw_map_rects(color)              │
│                                              │
│  update_all(camera)                          │
│  clear()                                     │
└──────────────────────────────────────────────┘
```

<br>

# Position Debug

```cpp
pos_debug::register_draw_position(player.pos, bn::color(31, 0, 0));   // red
```

### `Position` - Struct
**Stores** `x, y` `width, height` `offset_x, offset_y`
All Collision Objects and Sprites have a collision struct, a Physics body has three of them (`pos`, `shape_pos`, sprite's `pos`).

<br>

# Map Rects & Static Rects

```cpp
pos_debug::register_draw_map_rects(bn::color(15, 15, 31));
pos_debug::register_draw_rect(64, 32, 128, 96, bn::color(31, 31, 0));
```

### Map Rect
**Stores** `x_min, y_min, x_max, y_max, layer`
Stores multiple collision shapes without a class cheaply, ideal for map data.

### Draw Rect
**Signature**: `register_draw_rect(x_min, y_min, x_max, y_max, color)`
This function draws a square with the specified coordinates. Maybe useful.

<br>

# Loop

```cpp
pos_debug::update_all(Camera::instance());
```

Updates all debug shapes with the camera.

<br>

# Limits

| Constant       | Value | Meaning                                              |
|----------------|-------|------------------------------------------------------|
| `MAX_ENTRIES`  | 32    | Max debug shapes |
| `MAX_PALETTES` | 8     | Distinct colors |
| `MAX_AFFINES`  | 16    | Distinct `(width, height)` pairs for affine transformations |

**Buffer Overflow**: Will fail silently.

<br>

### Size Limit 128 x 128

Collision shapes larger than 128x128 in either direction are not supported.

<br>

### Color Palettes

Each new color will use a full 16 color palette. Make sure that is not a constraint.

<br>

### Affine Transformation

Each `(width, height)` combination will take up 1 of 32 affine transformation slots. If you use affine transformations heavily (scaling, shearing, rotation), be aware.
