#pragma once

#include "bn_fixed.h"
#include "collision_registry.h"
#include "position.h"

class Sprite;

class StaticBody {

public:
    Position pos;

    uint16_t layers;               // what am I
    uint8_t body_type = 0;         // user-defined tag for casting (0 = untyped)

    Sprite* sprite = nullptr;      // optional, nullptr if invisible

    void enable() { _enabled = true; }
    void disable() { _enabled = false; }
    bool is_enabled() const { return _enabled; }

    StaticBody(bn::fixed x, bn::fixed y, bn::fixed w, bn::fixed h,
               uint16_t layers);
    ~StaticBody();

private:
    bool _enabled = true;
};
