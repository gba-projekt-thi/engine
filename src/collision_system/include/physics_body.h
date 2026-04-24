#pragma once

#include "static_body.h"
#include "collision_shape.h"

class PhysicsBody : public StaticBody, public CollisionShape {

public:
    uint16_t block;                // what blocks me (move / collision)

    bn::fixed vel_x = 0;
    bn::fixed vel_y = 0;
    bn::fixed vel_max = 5;

    // direct positional move — resolves X and Y separately
    void move(bn::fixed dx, bn::fixed dy);

    // applies velocity, calls move() internally
    void move_velocity();

    void set_velocity(bn::fixed vx, bn::fixed vy);
    void inc_velocity(bn::fixed ax, bn::fixed ay);
    void dec_velocity(bn::fixed dx, bn::fixed dy);   // decelerates toward zero

    // called after move_velocity() — override to sync attached shapes/sprites
    virtual void move_attachments(bn::fixed /*x*/, bn::fixed /*y*/) {}

    // probe 1px outside the blocking rect (StaticBody), returns CollisionResult
    CollisionResult probe_bottom(uint16_t mask = 0xFFFF);
    CollisionResult probe_top(uint16_t mask = 0xFFFF);
    CollisionResult probe_left(uint16_t mask = 0xFFFF);
    CollisionResult probe_right(uint16_t mask = 0xFFFF);

    PhysicsBody(bn::fixed x, bn::fixed y, bn::fixed w, bn::fixed h,
                uint16_t layers, uint16_t mask, uint16_t block);
    virtual ~PhysicsBody();

private:
    bool _collides_with_map(bn::fixed nx, bn::fixed ny);
    bool _collides_with_body(StaticBody& body, bn::fixed nx, bn::fixed ny);
};
