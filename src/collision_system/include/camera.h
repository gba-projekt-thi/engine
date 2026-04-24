#pragma once

#include "bn_fixed.h"

class Camera {

public:
    static Camera& instance() {
        static Camera cam;
        return cam;
    }

    void init(bn::fixed world_w, bn::fixed world_h) {
        _world_width = world_w;
        _world_height = world_h;
    }

    void follow(bn::fixed target_x, bn::fixed target_y) {
        // clamp so screen never shows past map edges
        if(target_x < HALF_SCREEN_W) target_x = HALF_SCREEN_W;
        if(target_x > _world_width - HALF_SCREEN_W) target_x = _world_width - HALF_SCREEN_W;
        if(target_y < HALF_SCREEN_H) target_y = HALF_SCREEN_H;
        if(target_y > _world_height - HALF_SCREEN_H) target_y = _world_height - HALF_SCREEN_H;

        _x = target_x;
        _y = target_y;
    }

    // world → screen (Butano screen center = 0,0)
    bn::fixed to_screen_x(bn::fixed world_x) const { return world_x - _x; }
    bn::fixed to_screen_y(bn::fixed world_y) const { return world_y - _y; }

    // BG offset: shifts bg so camera world pos appears at screen center
    bn::fixed bg_x() const { return _world_width  / 2 - _x; }
    bn::fixed bg_y() const { return _world_height / 2 - _y; }

    bn::fixed x() const { return _x; }
    bn::fixed y() const { return _y; }

private:
    Camera() = default;

    static constexpr bn::fixed HALF_SCREEN_W = 120;   // 240 / 2
    static constexpr bn::fixed HALF_SCREEN_H = 80;    // 160 / 2

    bn::fixed _x = 0;
    bn::fixed _y = 0;
    bn::fixed _world_width = 0;
    bn::fixed _world_height = 0;
};