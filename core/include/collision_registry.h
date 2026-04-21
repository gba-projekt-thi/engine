#pragma once

#include "bn_fixed.h"

// forward declarations
class StaticBody;
class PhysicsBody;
class CollisionShape;

// =============================================================================
// MapRect — python-generated collision rect (from tilemap_data.h)
// =============================================================================

struct MapRect {
    bn::fixed x_min, y_min, x_max, y_max;
    uint16_t layers;
};

// =============================================================================
// CollisionHit / CollisionResult — returned by check_rect / check_point
// =============================================================================

struct CollisionHit {
    uint16_t layers;
    StaticBody* body;       // nullptr for map rects
    int map_rect_idx;       // -1 for bodies
};

struct CollisionResult {
    static constexpr int MAX_HITS = 8;
    CollisionHit hits[MAX_HITS] = {};
    int count = 0;

    uint16_t combined_layers() const {
        uint16_t result = 0;
        for(int i = 0; i < count; ++i) result |= hits[i].layers;
        return result;
    }
};

// =============================================================================
// CollisionRegistry — singleton, owns all collision data
// =============================================================================

class CollisionRegistry {

public:
    static CollisionRegistry& instance();

    // static bodies
    void register_body(StaticBody* b);
    void unregister_body(StaticBody* b);
    StaticBody** get_static_bodies() { return _static_bodies; }
    int get_static_body_count() const { return _static_body_count; }

    // physics bodies
    void register_physics_body(PhysicsBody* b);
    void unregister_physics_body(PhysicsBody* b);

    // collision shapes
    void register_shape(CollisionShape* s);
    void unregister_shape(CollisionShape* s);

    // map rects — bulk load or add one by one
    void load_map_rects(const MapRect* rects, int count);
    void add_map_rect(const MapRect& rect);
    const MapRect* get_map_rects() const { return _map_rects; }
    int get_map_rect_count() const { return _map_rect_count; }

    // one-shot collision queries against static bodies + map rects
    static CollisionResult check_rect(bn::fixed x, bn::fixed y,
                                      bn::fixed w, bn::fixed h,
                                      uint16_t mask,
                                      StaticBody* skip = nullptr);
    static CollisionResult check_point(bn::fixed px, bn::fixed py,
                                       uint16_t mask,
                                       StaticBody* skip = nullptr);

    // calls physics_update() on all physics bodies, then update() + resolve_overlaps on all shapes
    void update_all();

private:
    CollisionRegistry() = default;

    static constexpr int MAX_STATIC_BODIES = 64;
    StaticBody* _static_bodies[MAX_STATIC_BODIES] = {};
    int _static_body_count = 0;

    static constexpr int MAX_PHYSICS_BODIES = 16;
    PhysicsBody* _physics_bodies[MAX_PHYSICS_BODIES] = {};
    int _physics_body_count = 0;

    static constexpr int MAX_COLLISION_SHAPES = 32;
    CollisionShape* _collision_shapes[MAX_COLLISION_SHAPES] = {};
    int _collision_shape_count = 0;

    static constexpr int MAX_MAP_RECTS = 32;
    MapRect _map_rects[MAX_MAP_RECTS] = {};
    int _map_rect_count = 0;
};
