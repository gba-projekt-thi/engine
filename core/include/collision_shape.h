#pragma once

#include "bn_fixed.h"
#include "collision_registry.h"
#include "position.h"

class StaticBody;

class CollisionShape {

public:
    Position shape_pos;
    uint16_t mask;

    void shape_enable() { _shape_enabled = true; }
    void shape_disable();
    bool is_shape_enabled() const { return _shape_enabled; }

    // one-shot: what is this shape overlapping right now?
    CollisionResult get_collision() {
        return CollisionRegistry::check_rect(
            shape_pos.get_x(), shape_pos.get_y(), shape_pos.width, shape_pos.height, mask, _skip_body);
    }

    virtual void update() {}
    virtual void on_enter(uint16_t /*hit_layers*/, StaticBody* /*body*/) {}
    virtual void on_exit(uint16_t /*hit_layers*/, StaticBody* /*body*/) {}

    CollisionShape(bn::fixed x, bn::fixed y, bn::fixed w, bn::fixed h,
                   uint16_t initial_mask);
    virtual ~CollisionShape();

protected:
    StaticBody* _skip_body = nullptr;   // set by PhysicsBody to skip self

private:
    friend class CollisionRegistry;
    void _resolve_overlaps();
    void _clear_overlaps();

    bool _shape_enabled = true;
    CollisionHit _overlaps[CollisionResult::MAX_HITS] = {};
    int _overlap_count = 0;
};
