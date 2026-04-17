# 🧩 Core Scene System

This module provides the global infrastructure for state management. It acts as the "Engine Room" of the game, ensuring that switching between menus and gameplay is memory-safe and efficient for the GBA.

---

## 🚀 Key Features

* **Automatic Memory Management (RAII):** Uses `bn::unique_ptr` to ensure that when a scene is destroyed, all its associated resources (Sprites, Backgrounds, Palettes) are automatically cleared from VRAM.
* **Singleton Manager:** A central `SceneManager` to orchestrate the game flow.
* **Zero Dependencies:** This module is completely neutral. It doesn't know about RPGs or Platformers, making it the perfect foundation for multi-genre projects.

---

## 🏗️ Architecture

The `SceneManager` follows the **State Pattern**. It manages the lifecycle of `core::Scene` objects without needing to know their internal logic.



1.  **Current Scene:** The active game state being updated every frame.
2.  **Next Scene:** A buffer that holds the upcoming scene. When set, the manager destroys the current scene first (releasing memory) before initializing the new one.

---

## 💻 Usage

### 1. Create a Scene
Every game state must inherit from `core::Scene`.

```cpp
#include "core_scene.h"

class MainMenu : public core::Scene {
public:
    void update() override {
        // Handle menu logic here
    }
};
```

### 2. Trigger a Transition
Use the SceneManager from anywhere in your code to switch states.

```cpp
#include "core_scene_manager.h"

// The old scene will be deleted automatically!
core::SceneManager::instance().set_next_scene(bn::make_unique<MainMenu>());
```