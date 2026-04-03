#pragma once

#include "bn_fixed.h"
#include "collision_registry.h"

class Sprite;

class StaticBody {

public:
    bn::fixed x, y;                // world position (center of body)
    bn::fixed width, height;       // collision box size
    
    uint16_t layers;               // what am I
    //uint16_t mask;               // what I detect  (on_enter / on_exit) ⟶ moved PhysicsBody
    //uint16_t block;              // what blocks me (move / collision)   ⟶ moved PhysicsBody

    bool needs_physics_update = false;

    Sprite* sprite = nullptr;      // optional, nullptr if invisible
    bn::fixed sprite_offset_x = 0; // visual offset from collision box center
    bn::fixed sprite_offset_y = 0;

    StaticBody(bn::fixed x, bn::fixed y, bn::fixed w, bn::fixed h,
               uint16_t layers);
    ~StaticBody();
};