#pragma once

#include "bn_fixed.h"

// forward declarations
class StaticBody;
class PhysicsBody;

// =============================================================================
// MapRect — python-generated collision shape (from tilemap_data.h)
// =============================================================================

struct MapRect {
    bn::fixed x_min, y_min, x_max, y_max;
    uint16_t layers;
};

// =============================================================================
// CollisionRegistry — singleton, stores all StaticBody pointers
// =============================================================================

class CollisionRegistry {

public:
    static CollisionRegistry& instance();

    void register_body(StaticBody* b);
    void unregister_body(StaticBody* b);

    // calls physics_update() on every body where needs_physics_update == true
    void update_all();

    StaticBody** get_bodies() { return _bodies; }
    int get_count() const { return _count; }

private:
    CollisionRegistry() = default;

    static constexpr int MAX_BODIES = 64;
    StaticBody* _bodies[MAX_BODIES] = {};
    int _count = 0;
};