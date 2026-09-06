#include "collision_registry.h"
#include "bn_assert.h"
#include "static_body.h"
#include "collision_shape.h"
#include "physics_body.h"

CollisionRegistry& CollisionRegistry::instance() {
    static CollisionRegistry reg;
    return reg;
}

// =============================================================================
// Static bodies
// =============================================================================

void CollisionRegistry::register_body(StaticBody* b) {
    // Fail loudly rather than silently dropping the body: with
    // MAX_STATIC_BODIES derived from Cfg::Level::Limits, this should be
    // unreachable in practice. Hitting it means a level exceeds the
    // documented per-level object limits.
    BN_ASSERT(
        _static_body_count < MAX_STATIC_BODIES,
        "StaticBody registry full - increase "
        "Cfg::Level::Limits::MAX_STATIC_BODIES");
    _static_bodies[_static_body_count] = b;
    ++_static_body_count;
}

void CollisionRegistry::unregister_body(StaticBody* b) {
    for(int i = 0; i < _static_body_count; ++i) {
        if(_static_bodies[i] == b) {
            _static_bodies[i] = _static_bodies[_static_body_count - 1];
            --_static_body_count;
            return;
        }
    }
}

// =============================================================================
// Physics bodies
// =============================================================================

void CollisionRegistry::register_physics_body(PhysicsBody* b) {
    BN_ASSERT(
        _physics_body_count < MAX_PHYSICS_BODIES,
        "PhysicsBody registry full - increase "
        "Cfg::Level::Limits::MAX_PHYSICS_BODIES");
    _physics_bodies[_physics_body_count] = b;
    ++_physics_body_count;
}

void CollisionRegistry::unregister_physics_body(PhysicsBody* b) {
    for(int i = 0; i < _physics_body_count; ++i) {
        if(_physics_bodies[i] == b) {
            _physics_bodies[i] = _physics_bodies[_physics_body_count - 1];
            --_physics_body_count;
            return;
        }
    }
}

// =============================================================================
// Collision shapes
// =============================================================================

void CollisionRegistry::register_shape(CollisionShape* s) {
    BN_ASSERT(
        _collision_shape_count < MAX_COLLISION_SHAPES,
        "CollisionShape registry full - increase "
        "Cfg::Level::Limits::MAX_COLLISION_SHAPES");
    _collision_shapes[_collision_shape_count] = s;
    ++_collision_shape_count;
}

void CollisionRegistry::unregister_shape(CollisionShape* s) {
    for(int i = 0; i < _collision_shape_count; ++i) {
        if(_collision_shapes[i] == s) {
            _collision_shapes[i] = _collision_shapes[_collision_shape_count - 1];
            --_collision_shape_count;
            return;
        }
    }
}

// =============================================================================
// Map rects
// =============================================================================

void CollisionRegistry::load_map_rects(const MapRect* rects, int count) {
    _map_rect_count = 0;
    for(int i = 0; i < count && i < MAX_MAP_RECTS; ++i) {
        _map_rects[i] = rects[i];
        ++_map_rect_count;
    }
}

void CollisionRegistry::add_map_rect(const MapRect& rect) {
    if(_map_rect_count < MAX_MAP_RECTS) {
        _map_rects[_map_rect_count] = rect;
        ++_map_rect_count;
    }
}

// =============================================================================
// One-shot collision queries
// =============================================================================

CollisionResult CollisionRegistry::check_rect(bn::fixed x, bn::fixed y,
                                               bn::fixed w, bn::fixed h,
                                               uint16_t mask,
                                               StaticBody* skip) {
    CollisionResult result;
    CollisionRegistry& reg = instance();

    bn::fixed half_w = w / 2;
    bn::fixed half_h = h / 2;

    // check against static bodies
    for(int i = 0; i < reg._static_body_count && result.count < CollisionResult::MAX_HITS; ++i) {
        StaticBody* other = reg._static_bodies[i];
        if(other == skip) continue;
        if(!other->is_enabled()) continue;
        if((mask & other->layers) == 0) continue;

        bn::fixed b_half_w = other->pos.width / 2;
        bn::fixed b_half_h = other->pos.height / 2;
        bn::fixed bx = other->pos.get_x();
        bn::fixed by = other->pos.get_y();

        if(x - half_w < bx + b_half_w &&
           x + half_w > bx - b_half_w &&
           y - half_h < by + b_half_h &&
           y + half_h > by - b_half_h) {
            result.hits[result.count].layers = other->layers;
            result.hits[result.count].body = other;
            result.hits[result.count].map_rect_idx = -1;
            ++result.count;
        }
    }

    // check against map rects
    for(int i = 0; i < reg._map_rect_count && result.count < CollisionResult::MAX_HITS; ++i) {
        const MapRect& r = reg._map_rects[i];
        if((mask & r.layers) == 0) continue;

        if(x - half_w < r.x_max && x + half_w > r.x_min &&
           y - half_h < r.y_max && y + half_h > r.y_min) {
            result.hits[result.count].layers = r.layers;
            result.hits[result.count].body = nullptr;
            result.hits[result.count].map_rect_idx = i;
            ++result.count;
        }
    }

    return result;
}

CollisionResult CollisionRegistry::check_point(bn::fixed px, bn::fixed py,
                                                uint16_t mask,
                                                StaticBody* skip) {
    CollisionResult result;
    CollisionRegistry& reg = instance();

    // check against static bodies
    for(int i = 0; i < reg._static_body_count && result.count < CollisionResult::MAX_HITS; ++i) {
        StaticBody* other = reg._static_bodies[i];
        if(other == skip) continue;
        if(!other->is_enabled()) continue;
        if((mask & other->layers) == 0) continue;

        bn::fixed b_half_w = other->pos.width / 2;
        bn::fixed b_half_h = other->pos.height / 2;
        bn::fixed bx = other->pos.get_x();
        bn::fixed by = other->pos.get_y();

        if(px >= bx - b_half_w && px < bx + b_half_w &&
           py >= by - b_half_h && py < by + b_half_h) {
            result.hits[result.count].layers = other->layers;
            result.hits[result.count].body = other;
            result.hits[result.count].map_rect_idx = -1;
            ++result.count;
        }
    }

    // check against map rects
    for(int i = 0; i < reg._map_rect_count && result.count < CollisionResult::MAX_HITS; ++i) {
        const MapRect& r = reg._map_rects[i];
        if((mask & r.layers) == 0) continue;

        if(px >= r.x_min && px < r.x_max &&
           py >= r.y_min && py < r.y_max) {
            result.hits[result.count].layers = r.layers;
            result.hits[result.count].body = nullptr;
            result.hits[result.count].map_rect_idx = i;
            ++result.count;
        }
    }

    return result;
}

// =============================================================================
// Update loop
// =============================================================================

void CollisionRegistry::update_all() {
    // 1. all collision shapes: update() (input, AI, timers)
    for(int i = 0; i < _collision_shape_count; ++i) {
        _collision_shapes[i]->update();
    }

    // 2. all physics bodies: move_velocity() then move_attachments()
    for(int i = 0; i < _physics_body_count; ++i) {
        PhysicsBody* pb = _physics_bodies[i];
        pb->move_velocity();
        pb->move_attachments(pb->pos.x, pb->pos.y);
    }

    // 3. all collision shapes: resolve overlaps
    for(int i = 0; i < _collision_shape_count; ++i) {
        CollisionShape* s = _collision_shapes[i];
        if(s->is_shape_enabled()) {
            s->_resolve_overlaps();
        }
    }
}