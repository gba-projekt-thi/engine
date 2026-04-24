#pragma once

#include "bn_fixed.h"

struct Position {
    bn::fixed x = 0;
    bn::fixed y = 0;
    bn::fixed width = 0;
    bn::fixed height = 0;
    bn::fixed offset_x = 0;
    bn::fixed offset_y = 0;

    bn::fixed get_x() const { return x + offset_x; }
    bn::fixed get_y() const { return y + offset_y; }

    void move(bn::fixed new_x, bn::fixed new_y) {
        x = new_x;
        y = new_y;
    }
};
