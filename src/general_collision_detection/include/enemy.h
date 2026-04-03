#pragma once

#include "physics_body.h"

class Enemy : public PhysicsBody {

public:
    static constexpr uint16_t LAYERS = 0x0002;                  // what am I
    static constexpr uint16_t MASK   = 0x0000;                  // what I detect  (nothing — player detects me)
    static constexpr uint16_t BLOCK  = 0xFFFF & ~0x0001;        // what blocks me (walls, not player)

    Enemy(bn::fixed start_x, bn::fixed start_y, bn::fixed w, bn::fixed h,
          bn::fixed patrol_x1, bn::fixed patrol_x2, bn::fixed patrol_speed)
        : PhysicsBody(start_x, start_y, w, h, LAYERS, MASK, BLOCK),
          _patrol_x1(patrol_x1),
          _patrol_x2(patrol_x2),
          _patrol_speed(patrol_speed),
          _direction(1) {
        vel_max = patrol_speed;
    }

    void update() override {
        // patrol between two x coordinates
        if(x >= _patrol_x2) {
            _direction = -1;
        } else if(x <= _patrol_x1) {
            _direction = 1;
        }

        set_velocity(_patrol_speed * _direction, 0);
    }

    void on_enter(StaticBody& other) override {
    }

    void on_exit(StaticBody& other) override {
    }

private:
    bn::fixed _patrol_x1;       // left bound (world px)
    bn::fixed _patrol_x2;       // right bound (world px)
    bn::fixed _patrol_speed;
    int _direction;
};