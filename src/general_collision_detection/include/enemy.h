#pragma once

#include "physics_body.h"
#include "laser_weapon.h"
#include "collision_config.h"

class Enemy : public PhysicsBody {

public:
    static constexpr uint16_t MASK   = MASK_PLAYER;                  // what I detect  (player)
    static constexpr uint16_t BLOCK  = 0xFFFF & ~MASK_PLAYER;        // what blocks me (walls, not player)

    static constexpr uint16_t LASER_MASK = MASK_PLAYER;              // laser detects player
    static constexpr bn::fixed LASER_WIDTH  = 40;
    static constexpr bn::fixed LASER_HEIGHT = 1;
    static constexpr bn::fixed LASER_OFFSET_X = 28;             // center of laser from enemy center

    LaserWeapon laser_left;
    LaserWeapon laser_right;

    Enemy(bn::fixed start_x, bn::fixed start_y, bn::fixed w, bn::fixed h,
          bn::fixed patrol_x1, bn::fixed patrol_x2, bn::fixed patrol_speed)
        : PhysicsBody(start_x, start_y, w, h, MASK_ENEMY, MASK, BLOCK),
          laser_left(start_x, start_y, LASER_WIDTH, LASER_HEIGHT, LASER_MASK),
          laser_right(start_x, start_y, LASER_WIDTH, LASER_HEIGHT, LASER_MASK),
          _patrol_x1(patrol_x1),
          _patrol_x2(patrol_x2),
          _patrol_speed(patrol_speed),
          _direction(1) {
        body_type = TYPE_ENEMY;
        vel_max = patrol_speed;

        // set offsets so move() places them correctly
        laser_left.shape_pos.offset_x  = -LASER_OFFSET_X;
        laser_right.shape_pos.offset_x =  LASER_OFFSET_X;
    }

    void update() override {
        // patrol between two x coordinates
        if(pos.x >= _patrol_x2) {
            _direction = -1;
        } else if(pos.x <= _patrol_x1) {
            _direction = 1;
        }

        set_velocity(_patrol_speed * _direction, 0);

        // activate only the one matching direction
        if(_direction == -1) { laser_left.shape_enable(); laser_right.shape_disable(); }
        else { laser_left.shape_disable(); laser_right.shape_enable(); }
    }

    void move_attachments(bn::fixed x, bn::fixed y) override {
        laser_left.shape_pos.move(x, y);
        laser_right.shape_pos.move(x, y);
        if(laser_left.beam) laser_left.beam->pos.move(x, y);
        if(laser_right.beam) laser_right.beam->pos.move(x, y);
    }

private:
    bn::fixed _patrol_x1;       // left bound (world px)
    bn::fixed _patrol_x2;       // right bound (world px)
    bn::fixed _patrol_speed;
    int _direction;
};
