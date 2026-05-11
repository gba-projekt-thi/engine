# 📂 CORE SYSTEMS

This folder contains the foundational systems for the project. These modules are shared by both the **RPG** and **Platformer** teams to ensure hardware efficiency and consistent resource management.

### 📖 Documentation Reference
Refer to the following guides in the `/documentation` folder for implementation details:

* **[CollisionSystem.md](./documentation/CollisionSystem.md)**: Logic for collision detection.
* **[SceneSystem.md](./documentation/SceneSystem.md)**: Framework for managing game states and transitions.
* **[Sprites.md](./documentation/Sprites.md)**: Wrappers for sprite allocation.
* **[Tilemap.md](./documentation/Tilemap.md)**: Management of map data.

---

### 🛠️ Key Implementation Rules

1.  **RAII Compliance**: All hardware resources (VRAM, Palettes) must be managed through objects. Do not use raw pointers for engine assets.
2.  **Coordination**: Changes made to this folder affect both teams. Consult the lead dev before modifying core logic.

---
> **Note:** Proper use of these systems is mandatory to prevent VRAM overflows and memory leaks on the GBA hardware.