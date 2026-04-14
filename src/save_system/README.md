# Save & Load System

A lightweight, reusable save/load module for Game Boy Advance games built with [Butano](https://github.com/GValiente/butano). Designed to be included as a **Git submodule** in your game repository.

---

## Overview

This module provides a complete save/load system that persists game state to the GBA's 32 KB SRAM. It handles serialization, data integrity validation (CRC32), and multiple save slots out of the box.

The system is **template-based**, so each game defines its own state struct and the engine takes care of the rest — writing to hardware, verifying checksums, and managing slot layout.

Since Butano's `bn::sram` API only supports reading/writing a single object at offset 0 (no offset-based access), all slots are packed into a single `SramData<>` container struct that is read and written atomically.

```
┌──────────────────────────────────────────────────┐
│                  Your Game Code                   │
│  ┌────────────┐                                   │
│  │ MyGameState│──── defines ────┐                 │
│  └────────────┘                 │                 │
│                                 ▼                 │
│              SaveManager<MyGameState, 3>          │
│                 │          │          │            │
│              save()     load()    erase()         │
└──────────────┬──────────┬──────────┬──────────────┘
               │          │          │
       ┌───────▼──────────▼──────────▼───────┐
       │          save_system module          │
       │  ┌──────────┐  ┌──────────────────┐ │
       │  │SaveBuffer│  │ save_integrity   │ │
       │  │(serialize)│  │ (CRC32+validate) │ │
       │  └──────────┘  └──────────────────┘ │
       │  ┌──────────────────────────────┐   │
       │  │  SramData<> (all slots)      │   │
       │  └──────────────────────────────┘   │
       └─────────────────┬───────────────────┘
                         │
                  bn::sram::read()
                  bn::sram::write()
                         │
                ┌────────▼────────┐
                │   GBA SRAM      │
                │   (32 KB)       │
                └─────────────────┘
```

---

## Features

| Feature | Description |
|---|---|
| **Template-based API** | Define your own `GameState` struct — the manager adapts automatically. |
| **Multiple save slots** | Configurable number of slots (default: 3). Each slot is isolated in SRAM. |
| **CRC32 integrity checks** | Every save is checksummed. Corrupt data is detected on load. |
| **Magic number validation** | Prevents reading uninitialized SRAM as valid save data. |
| **Schema versioning** | Built-in version field for future data migration support. |
| **Zero dependencies** | Only requires Butano's `bn::sram`. No heap allocation, no STL. |
| **Submodule-ready** | Drop into any game repo as a Git submodule. |
| **Compile-time safety** | `static_assert` ensures your state fits in SRAM before you ever flash a ROM. |
| **25 passing tests** | Full test ROM covering CRC32, save/load roundtrips, multi-slot isolation, and edge cases. |

---

## Quick Start

### 1. Add the engine as a submodule to your game repo

```bash
cd your-game-repo
git submodule add https://github.com/gba-projekt-thi/engine.git extern/engine
git submodule update --init --recursive
```

### 2. Update your game's Makefile

Add the save system's `include/` and `src/` paths. Paths are relative to your game's `Makefile`:

```makefile
# Adjust the path depending on where you placed the submodule
ENGINE_SAVE := ../../extern/engine/src/save_system

SOURCES     := src $(ENGINE_SAVE)/src ../../extern/butano/common/src
INCLUDES    := include $(ENGINE_SAVE)/include ../../extern/butano/common/include
```

### 3. Define your game state

Create a **trivially-copyable** struct that holds everything you want to persist:

```cpp
// include/my_game_state.h
#pragma once
#include "bn_fixed.h"
#include <cstdint>

struct MyGameState
{
    bn::fixed player_x;
    bn::fixed player_y;
    uint8_t   current_level;
    uint16_t  score;
    uint32_t  inventory_flags;
};
```

### 4. Save and load

```cpp
#include "save_manager.h"
#include "my_game_state.h"

// Create a manager with 3 save slots
engine::save::SaveManager<MyGameState, 3> save_mgr;

// --- Save ---
MyGameState state;
state.player_x = player.x();
state.player_y = player.y();
state.score    = score;

engine::save::SaveResult result = save_mgr.save(0, state); // slot 0

// --- Load ---
MyGameState loaded;
result = save_mgr.load(0, loaded);

if(result == engine::save::SaveResult::OK)
{
    // Apply loaded state to your game
    player.set_position(loaded.player_x, loaded.player_y);
    score = loaded.score;
}

// --- Other operations ---
bool has_data = save_mgr.is_slot_used(0);   // check if slot contains valid data
save_mgr.erase(1);                          // wipe slot 1
```

### 5. Build and run

```bash
cd your-game-project && make
mgba-qt your_game.gba
```

---

## Core Concepts

### How SRAM Access Works

Butano's `bn::sram` API provides only two functions:

```cpp
template<class Type> void bn::sram::read(Type& destination);   // reads sizeof(Type) from offset 0
template<class Type> void bn::sram::write(const Type& source);  // writes sizeof(Type) to offset 0
```

There is **no offset parameter**. The entire SRAM is accessed as a single typed object. Therefore, the save system packs all slots into one `SramData<>` struct that is read/written atomically:

```cpp
// Internal layout — you don't interact with this directly
template<int MaxSlots, int PayloadSize>
struct SramData
{
    SaveSlot<PayloadSize> slots[MaxSlots];  // read/written as one block
};
```

Every `save()`, `load()`, `erase()`, and `is_slot_used()` call performs a full `bn::sram::read()`, modifies the relevant slot in RAM, then calls `bn::sram::write()` to persist the changes.

### SRAM Slot Layout

```
bn::sram::read/write operates on this entire block:

SramData<3, 512> = 3 × SaveSlot<512> = 3 × (16 + 512) = 1,584 Bytes

Offset 0x0000  ┌─────────────────────────┐
               │  slots[0]               │
               │  ├─ header   (16 B)     │  ← magic, version, checksum, size
               │  └─ payload (512 B)     │  ← your serialized GameState
               ├─────────────────────────┤
               │  slots[1]               │
               │  ├─ header   (16 B)     │
               │  └─ payload (512 B)     │
               ├─────────────────────────┤
               │  slots[2]               │
               │  ├─ header   (16 B)     │
               │  └─ payload (512 B)     │
               ├─────────────────────────┤
               │  (unused SRAM)          │
Offset 0x7FFF  └─────────────────────────┘  32 KB total
```

### SaveHeader

Every slot begins with a 16-byte header:

| Field | Type | Purpose |
|---|---|---|
| `magic` | `uint32_t` | Constant `0x53414247` ("GBAS"). Identifies initialized data. |
| `version` | `uint16_t` | Schema version. Increment when your `GameState` struct changes. |
| `slot_id` | `uint16_t` | Which slot this data belongs to. |
| `checksum` | `uint32_t` | CRC32 over the payload bytes. |
| `payload_size` | `uint32_t` | Actual number of payload bytes written. |

### Data Integrity Flow

```
  SAVE:                              LOAD:
  GameState                          SRAM
     │                                 │
     ▼                                 ▼
  serialize ──► payload bytes    bn::sram::read(sram_data)
     │                                 │
     ▼                                 ▼
  CRC32(payload) ──► header      validate magic ──► fail? → SLOT_EMPTY
     │                                 │
     ▼                                 ▼
  bn::sram::write(sram_data)     validate version ──► fail? → VERSION_MISMATCH
                                       │
                                       ▼
                                 CRC32(payload) == header.checksum?
                                       │              │
                                      yes             no
                                       │              │
                                       ▼              ▼
                                  deserialize    CHECKSUM_FAIL
                                       │
                                       ▼
                                   GameState ✓
```

### SaveResult Codes

| Code | Meaning |
|---|---|
| `OK` | Operation succeeded. |
| `SLOT_INVALID` | Slot index is out of range (< 0 or ≥ MaxSlots). |
| `SLOT_EMPTY` | No valid save data in this slot (bad magic or zero payload). |
| `CHECKSUM_FAIL` | Data is corrupt — CRC32 mismatch. |
| `VERSION_MISMATCH` | Save was written by a newer version of the game. |
| `SRAM_ERROR` | Reserved for future use. |
| `SERIALIZE_ERROR` | Serialization or deserialization failed (buffer overflow). |

### Template Parameters

```cpp
SaveManager<GameState, MaxSlots, BufferSize>
```

| Parameter | Default | Description |
|---|---|---|
| `GameState` | *(required)* | Your game's state struct. Must be trivially-copyable. |
| `MaxSlots` | `3` | Number of independent save slots. |
| `BufferSize` | `512` | Maximum payload size per slot in bytes. Must be ≥ `sizeof(GameState)`. |

Both constraints are checked at compile time via `static_assert`:
- `sizeof(SramData) <= 32768` — all slots must fit in SRAM
- `sizeof(GameState) <= BufferSize` — state must fit in the buffer

---

## File Structure

```
src/save_system/
├── include/
│   ├── save_header.h        # SaveHeader struct, magic/version constants
│   ├── save_buffer.h         # Typed read/write serialization buffer (template)
│   ├── save_integrity.h      # CRC32 and validation declarations
│   ├── save_slot.h            # SaveSlot<> and SramData<> container templates
│   └── save_manager.h        # Main template API (SaveManager + SaveResult)
└── src/
    └── save_integrity.cpp    # CRC32 implementation, header/checksum validation
```

---

## Test Suite

The system includes a dedicated test ROM at `src/save_system_test/` with **25 tests across 4 suites**:

| Suite | Tests | What is verified |
|---|---|---|
| **CRC32 & Integrity** | 6 | Determinism, different data produces different checksums, header validation, magic number rejection, checksum match/mismatch |
| **Save & Load** | 6 | Empty slot detection, `is_slot_used()`, save returns OK, roundtrip data correctness (all fields), erase |
| **Multiple Slots** | 8 | All 3 slots usable simultaneously, per-slot data isolation, erase doesn't affect neighbors, invalid index rejection |
| **Edge Cases** | 5 | Overwrite existing save, erase-then-load, negative index, `slot_count()`, `bn::fixed` precision survives roundtrip |

### Running the tests

```bash
# Build (inside dev container)
cd src/save_system_test && make

# Run interactively (on host)
mgba-qt src/save_system_test/save_test.gba

# Headless smoke test (CI)
../../test-rom.sh save_test.gba
```

Press **A** to advance through test suites. Press **START** on the results screen to rerun.

### SRAM Persistence Test (manual)

1. Run the test ROM — tests save data to SRAM
2. Close mGBA
3. Reopen the same ROM — mGBA restores SRAM from the `.sav` file
4. Verify that `is_slot_used()` returns `true` for previously saved slots

---

## Troubleshooting

### `static_assert` fails: "Save slots exceed available SRAM (32 KB)"

Your slot configuration uses more than 32,768 bytes. Reduce `MaxSlots` or `BufferSize`:

```cpp
// Before (too large):
SaveManager<HugeState, 10, 4096> mgr;  // 10 × (16 + 4096) = 41,120 bytes > 32 KB

// After (fits):
SaveManager<HugeState, 3, 2048> mgr;   // 3 × (16 + 2048) = 6,192 bytes ✓
```

### `static_assert` fails: "GameState is larger than BufferSize"

Your struct is bigger than the buffer. Increase `BufferSize`:

```cpp
struct BigState { uint8_t data[1024]; };

// Fix: increase buffer to fit
SaveManager<BigState, 3, 1024> mgr;
```

### `load()` returns `SLOT_EMPTY` on first run

This is expected. Fresh SRAM contains random data (or all `0xFF`), which won't match the magic number. Always handle this case:

```cpp
MyGameState state;
if(save_mgr.load(0, state) != engine::save::SaveResult::OK)
{
    // First run or corrupt data — use defaults
    state = default_state();
}
```

### `load()` returns `CHECKSUM_FAIL`

The save data in SRAM was corrupted (power loss during write, hardware defect, or incorrect emulator SRAM settings). The slot should be erased and the player notified:

```cpp
auto result = save_mgr.load(slot, state);
if(result == engine::save::SaveResult::CHECKSUM_FAIL)
{
    save_mgr.erase(slot);
    // Show "Save data corrupted" message to the player
}
```

### `load()` returns `VERSION_MISMATCH`

The save was written by a newer version of the game than the one currently running. This prevents reading data your code doesn't understand. To support old saves, increment `SAVE_VERSION` in `save_header.h` and add migration logic before deserialization.

### Save data doesn't persist in mGBA

Make sure SRAM is enabled in the emulator:
- **mGBA:** `Tools → Settings → Game Boy Advance → Save type: SRAM`
- Or ensure your ROM header declares SRAM support (Butano handles this by default)

### Runtime error: "output_sprites vector is full"

If you use `bn::sprite_text_generator` alongside the save system, make sure your `bn::vector<bn::sprite_ptr, N>` has a large enough capacity. Each text character creates one sprite. Use at least `128`–`256` for debug screens.

### Build errors: "save_header.h not found"

Your `INCLUDES` path in the Makefile doesn't point to the save system. Double-check the relative path:

```makefile
# Verify this path resolves correctly from your Makefile's location
INCLUDES := include ../../extern/engine/src/save_system/include
```

### Linker errors: undefined reference to `engine::save::crc32`

Your `SOURCES` path doesn't include the save system's `src/` directory:

```makefile
SOURCES := src ../../extern/engine/src/save_system/src
```

### Multiple games sharing the same cartridge SRAM

If you swap ROMs on the same cartridge, SRAM content from a different game may still be present. The magic number (`0x53414247`) prevents misinterpreting foreign data — `load()` will return `SLOT_EMPTY`.

For extra safety, you can change `SAVE_MAGIC` in `save_header.h` to a value unique to your game.

---

## Links

- [Butano Documentation](https://gvaliente.github.io/butano/)
- [Butano `bn::sram` Reference](https://gvaliente.github.io/butano/namespacebn_1_1sram.html)
- [GBATEK — GBA SRAM](https://problemkaputt.de/gbatek.htm#gbasram)
- [Engine Repository](https://github.com/gba-projekt-thi/engine)