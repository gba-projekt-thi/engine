#include "physics_body.h"
#include "sprite.h"
#include "collision_registry.h"
#include "bn_math.h"
#include "move_resolution.h"

// =============================================================================
// Construction
// =============================================================================

PhysicsBody::PhysicsBody(bn::fixed x, bn::fixed y, bn::fixed w, bn::fixed h,
                         uint16_t layers_, uint16_t mask_, uint16_t block_,
                         bn::fixed vel_max_)
    : StaticBody(x, y, w, h, layers_),
      CollisionShape(x, y, w, h, mask_),
      block(block_),
      vel_max(vel_max_) {
    body_type = 1;
    _skip_body = static_cast<StaticBody*>(this);
    CollisionRegistry::instance().register_physics_body(this);
}

PhysicsBody::~PhysicsBody() {
    CollisionRegistry::instance().unregister_physics_body(this);
}

// =============================================================================
// Velocity
// =============================================================================

void PhysicsBody::move_velocity() {
    // clamp velocity by magnitude (uniform top speed in every direction)
    bn::fixed speed_sq = vel_x * vel_x + vel_y * vel_y;
    bn::fixed max_sq   = vel_max * vel_max;

    if(speed_sq > max_sq) {
        bn::fixed speed = bn::fixed(bn::sqrt(speed_sq));
        vel_x = vel_x * vel_max / speed;
        vel_y = vel_y * vel_max / speed;
    }

    move(vel_x, vel_y);
}

void PhysicsBody::set_velocity(bn::fixed vx, bn::fixed vy) {
    vel_x = vx;
    vel_y = vy;
}

void PhysicsBody::inc_velocity(bn::fixed ax, bn::fixed ay) {
    vel_x += ax;
    vel_y += ay;
}

void PhysicsBody::dec_velocity(bn::fixed dx, bn::fixed dy) {
    // decelerate toward zero
    if(vel_x > 0) {
        vel_x -= dx;
        if(vel_x < 0) vel_x = 0;
    } else if(vel_x < 0) {
        vel_x += dx;
        if(vel_x > 0) vel_x = 0;
    }

    if(vel_y > 0) {
        vel_y -= dy;
        if(vel_y < 0) vel_y = 0;
    } else if(vel_y < 0) {
        vel_y += dy;
        if(vel_y > 0) vel_y = 0;
    }
}

// =============================================================================
// Movement — X and Y resolved independently for free wall-sliding
// =============================================================================

void PhysicsBody::move(bn::fixed dx, bn::fixed dy) {
    CollisionRegistry& reg = CollisionRegistry::instance();

    // Whether the body would overlap the map or any other enabled body at
    // (nx, ny).
    auto blocked_at = [this, &reg](bn::fixed nx, bn::fixed ny) {
        if(_collides_with_map(nx, ny)) {
            return true;
        }
        for(int i = 0; i < reg.get_static_body_count(); ++i) {
            StaticBody* other = reg.get_static_bodies()[i];
            if(other != static_cast<StaticBody*>(this) &&
               other->is_enabled() &&
               _collides_with_body(*other, nx, ny)) {
                return true;
            }
        }
        return false;
    };

    // --- resolve X, then Y: a blocked move goes up to the obstacle ---
    const AxisMove move_x = resolve_axis(
        pos.x, dx, [&](bn::fixed nx) { return blocked_at(nx, pos.y); });
    pos.x = move_x.position;
    if(move_x.blocked) {
        vel_x = 0;
    }

    const AxisMove move_y = resolve_axis(
        pos.y, dy, [&](bn::fixed ny) { return blocked_at(pos.x, ny); });
    pos.y = move_y.position;
    if(move_y.blocked) {
        vel_y = 0;
    }

    // --- sync collision shape position ---
    shape_pos.move(pos.x, pos.y);

    // --- sync sprite ---
    if(sprite) {
        sprite->pos.move(pos.x, pos.y);
    }
}

// =============================================================================
// Collision checks
// =============================================================================

bool PhysicsBody::_collides_with_map(bn::fixed nx, bn::fixed ny) {
    bn::fixed half_w = pos.width / 2;
    bn::fixed half_h = pos.height / 2;
    bn::fixed left   = nx - half_w;
    bn::fixed right  = nx + half_w;
    bn::fixed top    = ny - half_h;
    bn::fixed bottom = ny + half_h;

    CollisionRegistry& reg = CollisionRegistry::instance();
    const MapRect* rects = reg.get_map_rects();
    int rect_count = reg.get_map_rect_count();

    for(int i = 0; i < rect_count; ++i) {
        const MapRect& r = rects[i];
        if((block & r.layers) == 0) continue;

        if(left < r.x_max && right > r.x_min &&
           top  < r.y_max && bottom > r.y_min) {
            return true;
        }
    }
    return false;
}

bool PhysicsBody::_collides_with_body(StaticBody& body, bn::fixed nx, bn::fixed ny) {
    if((block & body.layers) == 0) return false;

    bn::fixed half_w = pos.width / 2;
    bn::fixed half_h = pos.height / 2;

    bn::fixed b_half_w = body.pos.width / 2;
    bn::fixed b_half_h = body.pos.height / 2;
    bn::fixed bx = body.pos.get_x();
    bn::fixed by = body.pos.get_y();

    return (nx - half_w < bx + b_half_w  &&
            nx + half_w > bx - b_half_w  &&
            ny - half_h < by + b_half_h  &&
            ny + half_h > by - b_half_h);
}

// =============================================================================
// Probes — 1px-thin rect outside the blocking rect (StaticBody)
// =============================================================================

CollisionResult PhysicsBody::probe_bottom(uint16_t mask_) {
    bn::fixed hh = pos.height / 2;
    return CollisionRegistry::check_rect(
        pos.x, pos.y + hh + 1, pos.width - 2, 1, mask_, _skip_body);
}

CollisionResult PhysicsBody::probe_top(uint16_t mask_) {
    bn::fixed hh = pos.height / 2;
    return CollisionRegistry::check_rect(
        pos.x, pos.y - hh - 1, pos.width - 2, 1, mask_, _skip_body);
}

CollisionResult PhysicsBody::probe_left(uint16_t mask_) {
    bn::fixed hw = pos.width / 2;
    return CollisionRegistry::check_rect(
        pos.x - hw - 1, pos.y, 1, pos.height - 2, mask_, _skip_body);
}

CollisionResult PhysicsBody::probe_right(uint16_t mask_) {
    bn::fixed hw = pos.width / 2;
    return CollisionRegistry::check_rect(
        pos.x + hw + 1, pos.y, 1, pos.height - 2, mask_, _skip_body);
}
