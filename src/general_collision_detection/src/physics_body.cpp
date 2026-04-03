#include "physics_body.h"
#include "sprite.h"
#include "collision_registry.h"
#include "tilemap_data.h"

// =============================================================================
// Construction
// =============================================================================

PhysicsBody::PhysicsBody(bn::fixed x, bn::fixed y, bn::fixed w, bn::fixed h,
                         uint16_t layers, uint16_t mask, uint16_t block)
    : StaticBody(x, y, w, h, layers), mask(mask), block(block) {
    needs_physics_update = true;
}

PhysicsBody::~PhysicsBody() {
}

// =============================================================================
// Update loop
// =============================================================================

void PhysicsBody::physics_update() {
    update();           // virtual — subclass behavior (keypad, AI, gravity)
    move_velocity();    // apply velocity → resolve collisions
}

void PhysicsBody::move_velocity() {
    // clamp velocity
    if(vel_x >  vel_max) vel_x =  vel_max;
    if(vel_x < -vel_max) vel_x = -vel_max;
    if(vel_y >  vel_max) vel_y =  vel_max;
    if(vel_y < -vel_max) vel_y = -vel_max;

    move(vel_x, vel_y);
}

// =============================================================================
// Movement — X and Y resolved independently for free wall-sliding
// =============================================================================

void PhysicsBody::move(bn::fixed dx, bn::fixed dy) {
    CollisionRegistry& reg = CollisionRegistry::instance();

    // --- resolve X ---
    bn::fixed nx = x + dx;
    bool blocked_x = _collides_with_map(nx, y);

    if(!blocked_x) {
        for(int i = 0; i < reg.get_count(); ++i) {
            StaticBody* other = reg.get_bodies()[i];
            if(other != this && _collides_with_body(*other, nx, y)) {
                blocked_x = true;
                break;
            }
        }
    }

    if(!blocked_x) {
        x = nx;
    } else {
        vel_x = 0;
    }

    // --- resolve Y ---
    bn::fixed ny = y + dy;
    bool blocked_y = _collides_with_map(x, ny);

    if(!blocked_y) {
        for(int i = 0; i < reg.get_count(); ++i) {
            StaticBody* other = reg.get_bodies()[i];
            if(other != this && _collides_with_body(*other, x, ny)) {
                blocked_y = true;
                break;
            }
        }
    }

    if(!blocked_y) {
        y = ny;
    } else {
        vel_y = 0;
    }

    // --- sync sprite ---
    if(sprite) {
        sprite->world_x = x + sprite_offset_x;
        sprite->world_y = y + sprite_offset_y;
    }

    // --- overlap callbacks ---
    _resolve_overlaps();
}

// =============================================================================
// Velocity helpers
// =============================================================================

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
// Collision checks
// =============================================================================

bool PhysicsBody::_collides_with_map(bn::fixed nx, bn::fixed ny) {
    bn::fixed half_w = width / 2;
    bn::fixed half_h = height / 2;
    bn::fixed left   = nx - half_w;
    bn::fixed right  = nx + half_w;
    bn::fixed top    = ny - half_h;
    bn::fixed bottom = ny + half_h;

    for(int i = 0; i < map_rect_count; ++i) {
        const MapRect& r = map_rects[i];
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

    bn::fixed half_w = width / 2;
    bn::fixed half_h = height / 2;

    bn::fixed b_half_w = body.width / 2;
    bn::fixed b_half_h = body.height / 2;

    return (nx - half_w < body.x + b_half_w  &&
            nx + half_w > body.x - b_half_w  &&
            ny - half_h < body.y + b_half_h  &&
            ny + half_h > body.y - b_half_h);
}

// =============================================================================
// Overlap enter / exit diffing
// =============================================================================

void PhysicsBody::_resolve_overlaps() {
    StaticBody* new_list[MAX_OVERLAPS] = {};
    int new_count = 0;

    bn::fixed half_w = width / 2;
    bn::fixed half_h = height / 2;

    CollisionRegistry& reg = CollisionRegistry::instance();

    for(int i = 0; i < reg.get_count() && new_count < MAX_OVERLAPS; ++i) {
        StaticBody* other = reg.get_bodies()[i];
        if(other == this) continue;
        if((mask & other->layers) == 0) continue;

        bn::fixed b_half_w = other->width / 2;
        bn::fixed b_half_h = other->height / 2;

        if(x - half_w < other->x + b_half_w  &&
           x + half_w > other->x - b_half_w  &&
           y - half_h < other->y + b_half_h  &&
           y + half_h > other->y - b_half_h) {
            new_list[new_count] = other;
            ++new_count;
        }
    }

    // on_exit: was overlapping, no longer is
    for(int i = 0; i < _overlap_count; ++i) {
        bool still = false;
        for(int j = 0; j < new_count; ++j) {
            if(_overlapping[i] == new_list[j]) { still = true; break; }
        }
        if(!still) {
            on_exit(*_overlapping[i]);
        }
    }

    // on_enter: newly overlapping
    for(int i = 0; i < new_count; ++i) {
        bool was = false;
        for(int j = 0; j < _overlap_count; ++j) {
            if(new_list[i] == _overlapping[j]) { was = true; break; }
        }
        if(!was) {
            on_enter(*new_list[i]);
        }
    }

    // store new list
    for(int i = 0; i < new_count; ++i) {
        _overlapping[i] = new_list[i];
    }
    _overlap_count = new_count;
}

// =============================================================================
// Probes — 3 points, 1px outside body edge, returns combined layers hit
// =============================================================================

uint16_t PhysicsBody::_probe_point(bn::fixed px, bn::fixed py) {
    uint16_t result = 0;

    // map rects
    for(int i = 0; i < map_rect_count; ++i) {
        const MapRect& r = map_rects[i];
        if(px >= r.x_min && px < r.x_max &&
           py >= r.y_min && py < r.y_max) {
            result |= r.layers;
        }
    }

    // registry bodies
    CollisionRegistry& reg = CollisionRegistry::instance();
    for(int i = 0; i < reg.get_count(); ++i) {
        StaticBody* other = reg.get_bodies()[i];
        if(other == this) continue;

        bn::fixed bw = other->width / 2;
        bn::fixed bh = other->height / 2;
        if(px >= other->x - bw && px < other->x + bw &&
           py >= other->y - bh && py < other->y + bh) {
            result |= other->layers;
        }
    }

    return result;
}

uint16_t PhysicsBody::probe_bottom() {
    bn::fixed hw = width / 2;
    bn::fixed hh = height / 2;
    bn::fixed py = y + hh + 1;
    return _probe_point(x - hw + 1, py) |
           _probe_point(x,          py) |
           _probe_point(x + hw - 1, py);
}

uint16_t PhysicsBody::probe_top() {
    bn::fixed hw = width / 2;
    bn::fixed hh = height / 2;
    bn::fixed py = y - hh - 1;
    return _probe_point(x - hw + 1, py) |
           _probe_point(x,          py) |
           _probe_point(x + hw - 1, py);
}

uint16_t PhysicsBody::probe_left() {
    bn::fixed hw = width / 2;
    bn::fixed hh = height / 2;
    bn::fixed px = x - hw - 1;
    return _probe_point(px, y - hh + 1) |
           _probe_point(px, y)          |
           _probe_point(px, y + hh - 1);
}

uint16_t PhysicsBody::probe_right() {
    bn::fixed hw = width / 2;
    bn::fixed hh = height / 2;
    bn::fixed px = x + hw + 1;
    return _probe_point(px, y - hh + 1) |
           _probe_point(px, y)          |
           _probe_point(px, y + hh - 1);
}