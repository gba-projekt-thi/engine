#include "physics_body.h"
#include "sprite.h"
#include "collision_registry.h"
#include "bn_math.h"

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

    // --- resolve X ---
    bn::fixed nx = pos.x + dx;
    bool blocked_x = _collides_with_map(nx, pos.y);

    if(!blocked_x) {
        for(int i = 0; i < reg.get_static_body_count(); ++i) {
            StaticBody* other = reg.get_static_bodies()[i];
            if(other != static_cast<StaticBody*>(this) &&
               other->is_enabled() &&
               _collides_with_body(*other, nx, pos.y)) {
                blocked_x = true;
                break;
            }
        }
    }

    if(!blocked_x) {
        pos.x = nx;
    } else {
        vel_x = 0;
    }

    // --- resolve Y ---
    bn::fixed ny = pos.y + dy;
    bool blocked_y = _collides_with_map(pos.x, ny);

    if(!blocked_y) {
        for(int i = 0; i < reg.get_static_body_count(); ++i) {
            StaticBody* other = reg.get_static_bodies()[i];
            if(other != static_cast<StaticBody*>(this) &&
               other->is_enabled() &&
               _collides_with_body(*other, pos.x, ny)) {
                blocked_y = true;
                break;
            }
        }
    }

    if(!blocked_y) {
        pos.y = ny;
    } else {
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
