# Scene Manager Integration Test

This project tests the `core::SceneManager` from the engine core as described in [Issue #29](https://github.com/gba-projekt-thi/engine/issues/29).

## Goal

Verify that the **RAII pattern** works correctly:  
When switching scenes, all associated objects (sprites, embedded game objects like `Player`) must be physically removed from memory **before** the new scene is initialized.

---

## Scenes

| Scene | Description |
|---|---|
| `TitleScene` | Start scene. Displays a title screen. START switches to `LevelScene`. |
| `LevelScene` | Game scene. Owns a `Player` object (RAII proof). START switches back to `TitleScene`. |

`Player` is a pure test object within `LevelScene` that only produces `BN_LOG` output in its constructor and destructor.

---

## Structure of `main.cpp`

`main()` is intentionally minimal — only the SceneManager lifecycle runs here:

```cpp
int main()
{
    bn::core::init();
    // ...

    core::SceneManager::instance().set_next_scene(bn::make_unique<TitleScene>(gen));

    while(true)
    {
        core::SceneManager::instance().update();
        bn::core::update();
    }
}
```

---

## Verification in mGBA

1. Build the ROM (see below) and load it in **mGBA**.
2. Open **Tools → Message Log**.
3. Press **START** to switch between scenes.

### Expected Log Output (TitleScene → LevelScene)

```
CONSTRUCTOR: TitleScene initialized.
--- START pressed ---
CONSTRUCTOR: Player created.
CONSTRUCTOR: LevelScene initialized.
DESTRUCTOR:  TitleScene deleted.
```

### Expected Log Output (LevelScene → TitleScene)

```
CONSTRUCTOR: TitleScene initialized.
DESTRUCTOR:  Player removed from memory.
DESTRUCTOR:  LevelScene deleted.
```

> **Important:** The `DESTRUCTOR` logs of the old scene must appear **after** the `CONSTRUCTOR` log of the new scene.  
> This is the expected behavior of the `SceneManager` (`set_next_scene` constructs first, then `update()` deletes the old scene via RAII).

---

## Acceptance Criteria (from Issue #4)

- [x] `main.cpp` is limited to SceneManager initialization
- [x] Switching from `TitleScene` → `LevelScene` and back works without crashing
- [x] The mGBA log confirms the destruction of the old scene and its `Player` object
- [ ] VRAM viewer in mGBA shows that old tiles disappear and are replaced by new ones (no "tile stacking")

---

## Build

```bash
cd src/scene_manager_test
make
```

> Prerequisite: Butano dev container with `DEVKITARM` and `WONDERFUL_TOOLCHAIN`.  
> See the [main README](../../README.md) for setup instructions.
