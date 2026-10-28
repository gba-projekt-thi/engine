#pragma once

#include "bn_fixed.h"

// Result of moving a body along one axis.
struct AxisMove {
    bn::fixed position;
    bool blocked = false;  // something was in the way (velocity -> 0)
};

// Moves along one axis from `from` by `delta`. `blocked(position)` tells
// whether the body would overlap something there. When the full move is free
// the body goes all the way; otherwise it advances in steps of at most 1 px up
// to the obstacle, so it ends less than 1 px before it (in reach of the 1 px
// probes) instead of stopping up to `delta` short of it. The steps only run on
// a blocked move, so a free move costs one check as before.
//
// Host-tested in the platformer's tests/host/test_move_resolution.cpp.
template <typename Blocked>
auto resolve_axis(bn::fixed from, bn::fixed delta, Blocked blocked) -> AxisMove {
    if (delta == 0 || !blocked(from + delta)) {
        return {from + delta, false};
    }

    const bn::fixed one = delta > 0 ? bn::fixed(1) : bn::fixed(-1);
    bn::fixed position = from;
    bn::fixed remaining = delta;
    while (remaining != 0) {
        const bn::fixed step =
            (remaining > 0 ? remaining > one : remaining < one) ? one : remaining;
        if (blocked(position + step)) {
            break;
        }
        position += step;
        remaining -= step;
    }
    return {position, true};
}
