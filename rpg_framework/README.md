# 📸 RPG Camera Module (Butano Framework)

## Overview
This module provides a **Singleton-based Camera Controller** specifically designed for 2D top-down RPGs on the Game Boy Advance. It handles smooth target tracking, automatic map boundary clamping, and coordinate projection.

## Features
* **Target Tracking:** Keeps a specific world entity (e.g., the player) centered on the screen.
* **Boundary Clamping:** Prevents the camera from scrolling past the edges of your map assets.
* **World-to-Screen Projection:** Helper functions to convert absolute world coordinates into relative screen coordinates (essential for UI/HUD).
* **Small Map Support:** Gracefully handles maps that are smaller than the GBA screen resolution (240x160).

### Camera Smoothing (Lerping)
The camera follows the target smoothly using linear interpolation. You can adjust the "snappiness" of the camera in the `follow` function.

```cpp
// Default: Uses 0.1 (Smooth and organic)
rpg::Camera::instance().follow(player.pos());

// Faster: Uses 0.2 (Snappier, good for action)
rpg::Camera::instance().follow(player.pos(), 0.2);

// Slower: Uses 0.05 (Very cinematic/heavy feel)
rpg::Camera::instance().follow(player.pos(), 0.05);
```

---

## Quick Start

### 1. Initialization
In your level loading logic, initialize the camera with the dimensions of your current map (in pixels).

```cpp
#include "rpg_engine.h"

void load_level() {
    // Example: Initialize for a 512x512 pixel map
    rpg::Camera::instance().init(512, 512);
}
```
Every Sprite or Background that should move with the camera must be linked to the camera pointer. Otherwise, objects will "drift" or stay static on the screen.

### 2. Linking Visuals (Crucial Step)

// Link your background
my_bg.set_camera(rpg::Camera::instance().ptr());

// Link your sprites
my_player_sprite.set_camera(rpg::Camera::instance().ptr());

### 3. The Update Loop
In your main game loop, update the camera position by passing the current world position of your target.

```cpp
while(true) {
    // 1. Move your player (logic)
    // 2. Make camera follow the player
    rpg::Camera::instance().follow(player.pos());

    bn::core::update();
}
```

## Core Concepts
World Space vs. Screen Space
World Space: The coordinate on your actual map (e.g., Pixel 400, 400).

Screen Space: The coordinate on the physical GBA screen (0-240, 0-160).

To place a UI element (like a "!" icon) above an NPC, use the to_screen utility:

bn::fixed_point screen_pos = rpg::Camera::instance().to_screen(npc.world_pos());
ui_icon_sprite.set_position(screen_pos);
Automatic Clamping
The camera uses bn::clamp to ensure the viewport never shows the area outside your map.

If your map is larger than 240x160: The camera scrolls and stops at the edges.

If your map is smaller than 240x160: The camera stays locked to the center of the map.

---> = Data Flow (Position, Speed)
 <-> = Camera Link (sync movement)
┌───────────────────────────────────────────────────────────────────┐
│                          CAMERA MODULE                            │
│                  (rpg::Camera Singleton)                          │
├───────────────────────────────────────────────────────────────────┤
│                                                                   │
│  [ Input ]              [ Logic: follow() ]          [ Output ]   │
│                                                                   │
│  Hero Pos  ─────┐        ┌─────────────────┐        ┌───────────┐ │
│  (World X,Y)    │        │ 1. Clamp to Map │        │ GBA Screen│ │
│                 ├───────▶│ 2. Apply Lerp  ├───────▶│ Viewport  │ │
│  Lerp Speed   ──┘        │ 3. Update Ptr   │        └───────────┘ │
│                          └────────┬────────┘             ▲        │
│                                   │                      │        │
│                                   ▼                      │        │
│                         ┌─────────────────┐              │        │
│                         │ bn::camera_ptr  │──────────────┘        │
│                         │ (Internal GBA)  │                       │
│                         └─────────────────┘                       │
│                                  ▲                                │
│                                  │ (link via .ptr())              │
│          ┌───────────────────────┴──────────────────────┐         │
│          │                       │                      │         │
│   ┌──────┴──────┐         ┌──────┴──────┐        ┌──────┴──────┐  │
│   │ Map Sprite  │         │ Hero Sprite │        │ NPC Sprites │  │
│   │ (uses cam)  │◀────────│(provides pos)────────▶ (uses cam) │  │
│   └─────────────┘         └─────────────┘        └─────────────┘  │
│                                                                   │
└───────────────────────────────────────────────────────────────────┘

## Troubleshooting
**Q: My player moves, but the map stays still!**

Make sure you called my_bg.set_camera(rpg::Camera::instance().ptr()).

**Q: My player disappears when I move!**

Ensure the player sprite is also linked to the camera pointer using set_camera().

**Q: The camera jitters at the map edges.**

Check if the dimensions passed to init(w, h) match your actual asset size exactly.

# ⚔️ RPG Level Framework

This module extends the core system specifically for Top-Down RPG mechanics. It bridges the gap between the global `SceneManager` and the specialized `rpg::Camera`.

---

## ✨ Features

* **Automated Camera:** Every `rpg::LevelScene` handles the `rpg::Camera` initialization and updates automatically.
* **Focus Tracking:** By simply updating the `_camera_focus` point, the camera will smoothly follow your target (Player, NPC, or Cutscene event).
* **Boilerplate Reduction:** Provides helper methods to link Butano's raw pointers to the camera system in a single line.

---

## 🛠️ How to Build a Level

Instead of inheriting from `core::Scene`, the RPG team should use `rpg::LevelScene`. It uses the **Template Method Pattern** to ensure camera logic always runs after your gameplay logic to prevent "frame lag".

### Example implementation:

```cpp
#include "rpg_level_scene.h"

class ForestLevel : public rpg::LevelScene {
public:
    // Pass the map dimensions to the constructor
    ForestLevel() : rpg::LevelScene(1024, 1024) {
        // Use helpers to link assets to the camera
        setup_background(_map);
        setup_sprite(_hero);
    }

    void update_logic() override {
        // 1. Handle your gameplay (input, collisions)
        // ...

        // 2. Tell the framework what to look at
        _camera_focus = _hero.position();
    }

private:
    bn::regular_bg_ptr _map;
    bn::sprite_ptr _hero;
};
```

## 📖 API Reference

### Protected Members (Accessible by your Level)
* _camera_focus (bn::fixed_point): The target coordinate for the camera.

* setup_background(bg): Registers a background with the camera system.

* setup_sprite(sprite): Registers a sprite with the camera system.

## Virtual Methods
* update_logic(): Implement your frame-by-frame gameplay here. Do not override update(), as it is marked final to protect the camera synchronization.