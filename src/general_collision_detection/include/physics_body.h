#pragma once

#include "static_body.h"

class PhysicsBody : public StaticBody {

public:
    uint16_t mask;                 // what I detect  (on_enter / on_exit)
    uint16_t block;                // what blocks me (move / collision)

    bn::fixed vel_x = 0;
    bn::fixed vel_y = 0;
    bn::fixed vel_max = 5;

    // called by CollisionRegistry::update_all() if `needs_physics_update` is true
    void physics_update();

    // override in subclass for per-frame behavior (keypad, AI, gravity)
    virtual void update() {}

    // direct positional move — resolves X and Y separately
    void move(bn::fixed dx, bn::fixed dy);

    // applies velocity, calls move() internally, get's called from physics_update()
    void move_velocity();

    void set_velocity(bn::fixed vx, bn::fixed vy);
    void inc_velocity(bn::fixed ax, bn::fixed ay);
    void dec_velocity(bn::fixed dx, bn::fixed dy);   // decelerates toward zero

    // probe 3 points 1px outside body edge, return combined layers of anything hit
    uint16_t probe_bottom();
    uint16_t probe_top();
    uint16_t probe_left();
    uint16_t probe_right();

    // overlap callbacks — override in subclass
    virtual void on_enter([[maybe_unused]] StaticBody& other) {}
    virtual void on_exit([[maybe_unused]] StaticBody& other) {}

    PhysicsBody(bn::fixed x, bn::fixed y, bn::fixed w, bn::fixed h,
                uint16_t layers, uint16_t mask, uint16_t block);
    virtual ~PhysicsBody();

private:
    static constexpr int MAX_OVERLAPS = 16;
    StaticBody* _overlapping[MAX_OVERLAPS] = {};
    int _overlap_count = 0;

    bool _collides_with_map(bn::fixed nx, bn::fixed ny);
    bool _collides_with_body(StaticBody& body, bn::fixed nx, bn::fixed ny);
    void _resolve_overlaps();
    uint16_t _probe_point(bn::fixed px, bn::fixed py);
};