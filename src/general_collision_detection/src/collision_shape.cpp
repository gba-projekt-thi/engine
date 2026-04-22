#include "collision_shape.h"
#include "collision_registry.h"
#include "static_body.h"

// =============================================================================
// Construction
// =============================================================================

CollisionShape::CollisionShape(bn::fixed x, bn::fixed y, bn::fixed w, bn::fixed h,
                               uint16_t initial_mask)
    : shape_pos{x, y, w, h}, mask(initial_mask) {
    CollisionRegistry::instance().register_shape(this);
}

CollisionShape::~CollisionShape() {
    CollisionRegistry::instance().unregister_shape(this);
}

// =============================================================================
// Enable / Disable
// =============================================================================

void CollisionShape::shape_disable() {
    if(_shape_enabled) {
        _shape_enabled = false;
        _clear_overlaps();
    }
}

// =============================================================================
// Clear overlaps — fires on_exit for everything currently overlapping
// =============================================================================

void CollisionShape::_clear_overlaps() {
    for(int i = 0; i < _overlap_count; ++i) {
        on_exit(_overlaps[i].layers, _overlaps[i].body);
    }
    _overlap_count = 0;
}

// =============================================================================
// Overlap enter / exit diffing
// =============================================================================

void CollisionShape::_resolve_overlaps() {
    CollisionResult result = CollisionRegistry::check_rect(
        shape_pos.get_x(), shape_pos.get_y(),
        shape_pos.width, shape_pos.height,
        mask, _skip_body);

    bool is_new[CollisionResult::MAX_HITS];
    for(int i = 0; i < result.count; ++i) is_new[i] = true;

    // check old against new — match = not new, no match = on_exit
    for(int i = 0; i < _overlap_count; ++i) {
        bool found = false;
        for(int j = 0; j < result.count; ++j) {
            if(_overlaps[i].body == result.hits[j].body &&
               _overlaps[i].map_rect_idx == result.hits[j].map_rect_idx) {
                is_new[j] = false;
                found = true;
                break;
            }
        }
        if(!found) on_exit(_overlaps[i].layers, _overlaps[i].body);
    }

    // anything still marked new = on_enter
    for(int i = 0; i < result.count; ++i) {
        if(is_new[i]) on_enter(result.hits[i].layers, result.hits[i].body);
    }

    // copy result into overlap list
    _overlap_count = result.count;
    for(int i = 0; i < result.count; ++i) {
        _overlaps[i] = result.hits[i];
    }
}
