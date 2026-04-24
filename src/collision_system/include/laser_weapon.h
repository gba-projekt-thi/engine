#pragma once

#include "collision_shape.h"
#include "sprite.h"

class LaserWeapon : public CollisionShape {

public:
    Sprite* beam = nullptr;

    LaserWeapon(bn::fixed x, bn::fixed y, bn::fixed w, bn::fixed h, uint16_t initial_mask)
        : CollisionShape(x, y, w, h, initial_mask) {
        shape_disable();
    }

    void on_enter(uint16_t /*hit_layers*/, StaticBody* /*body*/) override {
        if(beam) {
            beam->enable();
        }
    }

    void on_exit(uint16_t /*hit_layers*/, StaticBody* /*body*/) override {
        if(beam) {
            beam->disable();
        }
    }
};
