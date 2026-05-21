#pragma once

#include "bn_color.h"
#include "bn_fixed.h"
#include "position.h"

class Camera;

namespace pos_debug {

#ifdef POS_DEBUG

    void register_draw_position(const Position& pos, bn::color color);
    void register_draw_positions(const Position* positions, int count, bn::color color);
    void register_draw_rect(bn::fixed x_min, bn::fixed y_min, bn::fixed x_max, bn::fixed y_max, bn::color color);
    void register_draw_map_rects(bn::color color);
    void update_all(const Camera& camera);
    void clear();

#else

    inline void register_draw_position(const Position&, bn::color) {}
    inline void register_draw_positions(const Position*, int, bn::color) {}
    inline void register_draw_rect(bn::fixed, bn::fixed, bn::fixed, bn::fixed, bn::color) {}
    inline void register_draw_map_rects(bn::color) {}
    inline void update_all(const Camera&) {}
    inline void clear() {}

#endif

} // namespace pos_debug
