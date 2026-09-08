#pragma once

#include "bn_fixed.h"

class Camera {

public:
    static Camera& instance() {
        static Camera cam;
        return cam;
    }

    // world_w/world_h are the full extents of the playable world, centered
    // at world coordinate (0,0) - i.e. the world spans
    // [-world_w/2, +world_w/2] x [-world_h/2, +world_h/2]. This matches the
    // coordinate convention already used everywhere else (Position,
    // PhysicsBody, level spawn/door/platform data).
    void init(bn::fixed world_w, bn::fixed world_h) {
        _world_width = world_w;
        _world_height = world_h;
    }

    void follow(bn::fixed target_x, bn::fixed target_y) {
        // How far the camera center may travel from world (0,0) while still
        // keeping the screen fully inside the world bounds.
        bn::fixed max_x = _world_width / 2 - HALF_SCREEN_W;
        bn::fixed max_y = _world_height / 2 - HALF_SCREEN_H;

        // World smaller than (or equal to) the screen in that axis -> the
        // camera can't move at all on that axis, stays centered on (0,0).
        if (max_x < 0) max_x = 0;
        if (max_y < 0) max_y = 0;

        if (target_x < -max_x) target_x = -max_x;
        if (target_x > max_x) target_x = max_x;
        if (target_y < -max_y) target_y = -max_y;
        if (target_y > max_y) target_y = max_y;

        _x = target_x;
        _y = target_y;
    }

    // world → screen (Butano screen center = 0,0)
    bn::fixed to_screen_x(bn::fixed world_x) const { return world_x - _x; }
    bn::fixed to_screen_y(bn::fixed world_y) const { return world_y - _y; }

    // BG offset: the background is authored with its own center at world
    // (0,0), so panning it by -camera moves it opposite to the camera pan.
    bn::fixed bg_x() const { return -_x; }
    bn::fixed bg_y() const { return -_y; }

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