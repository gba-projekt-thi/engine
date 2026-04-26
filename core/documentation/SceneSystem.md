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

# 💾 Persistence Pattern: Handling Data across Scenes

Since our `core::SceneManager` uses **RAII**, every scene is completely destroyed during a transition to save VRAM and RAM. To keep data like HP or Progress alive, we use a **Global Singleton Pattern**.

---

## 1. The Data Container (Game Team)
Each team defines its own `GameData` struct in their respective folders. This object lives in the GBA's static memory and is never destroyed.

```cpp
// rpg/include/rpg_game_data.h
namespace [TeamName] {
    struct GameData {
        int hp = 100;
        int deaths = 0;

        static GameData& instance() {
            static GameData data; // Persistent static instance
            return data;
        }
    };
}
```

## 2. Scene Integration (How to use it)
To persist data, you must "pull" it when a scene starts and "push" it back before it is destroyed by the SceneManager.

* **PULL** (Constructor/Init): Load the current "Truth" from the global store.

* **PUSH** (Destructor): Save the current state back to the global store before the scene is wiped from memory.

```cpp
// Example: src/level_scene.cpp
using namespace [TeamName];

LevelScene::LevelScene() {
    // PULL: Initialize local variables from global store
    this->_local_health = GameData::instance().health;
}

LevelScene::~LevelScene() {
    // PUSH: Save local state to global store
    // This is called by the SceneManager right before destruction
    GameData::instance().health = this->_local_health;
}
```

## 3. The Engine's Role (SceneManager)
The SceneManager remains generic. It doesn't know about hp or deaths. It simply triggers the lifecycle that makes the "Push/Pull" possible.

```cpp
// core/src/scene_manager.cpp
void SceneManager::update() {
    if(_next_scene) {
        // 1. Trigger the Destructor of the current scene (Push happens here)
        _current_scene.reset(); 

        // 2. Load the new scene (Pull happens in its constructor)
        _current_scene = bn::move(_next_scene);
    }
}
```

## 🔄 The Lifecycle Flow
1. Scene A is active: Player loses health.

2. Transition triggered: SceneManager::set_next_scene() is called.

3. Destruction: SceneManager calls reset() on Scene A.

4. Data Save: Scene A's destructor writes health to GameData.

5. Creation: Scene B is constructed.

6. Data Load: Scene B's constructor reads health from GameData.

## 🛡️ Why we do this
- Decoupling: The Engine (core) doesn't need to know anything about the RPG's data.

- Memory Safety: We don't have "dangling pointers" because we only store raw numbers (int, bool), not the objects themselves.

- Flexibility: The Platformer team can have a totally different GameData struct than the RPG team.