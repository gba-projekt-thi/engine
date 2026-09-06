#pragma once
#include <cstdint>
#include "bn_fixed.h"

namespace Cfg_Core {

namespace Game::Limits {
inline constexpr unsigned int PLATFORMS = 32;
inline constexpr unsigned int PLATFORM_BODIES = 32;
inline constexpr unsigned int TRIGGERS = 16;
inline constexpr unsigned int TOTAL_TRAPS = 32;

// Objects that register with the collision system outside of per-level
// data but are always present while a level is active: the player and
// the level's door.
inline constexpr unsigned int PERSISTENT_BODIES = 2;

// Worst-case simultaneous StaticBody registrations for a single level:
// platform bodies + triggers + traps + player + door.
//
// Derived from the limits above (rather than hardcoded in
// CollisionRegistry) so the registry's array capacity can never
// silently drift out of sync with what level data is actually allowed
// to contain.
inline constexpr unsigned int MAX_STATIC_BODIES =
    PLATFORM_BODIES + TRIGGERS + TOTAL_TRAPS + PERSISTENT_BODIES;

// Worst-case simultaneous PhysicsBody registrations. Platforms are
// plain StaticBody instances (not PhysicsBody), so they are excluded
// here - see level_manager.cpp's _load_platforms().
inline constexpr unsigned int MAX_PHYSICS_BODIES =
    TRIGGERS + TOTAL_TRAPS + PERSISTENT_BODIES;

// Every PhysicsBody (Player, Door, Trigger, all traps) is also a
// CollisionShape, so the worst case matches MAX_PHYSICS_BODIES exactly.
inline constexpr unsigned int MAX_COLLISION_SHAPES = MAX_PHYSICS_BODIES;
}  // namespace Level::Limits

}