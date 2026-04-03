#pragma once

#include "bn_sprite_ptr.h"
#include "bn_fixed.h"
#include "bn_utility.h"
#include "camera.h"
#include "sprite_registry.h"

class Sprite {

public:
    bn::fixed world_x;
    bn::fixed world_y;

    Sprite(bn::sprite_ptr spr, bn::fixed wx, bn::fixed wy)
        : world_x(wx), world_y(wy), _sprite(bn::move(spr)) {
        SpriteRegistry::instance().register_sprite(this);
    }

    void sync(const Camera& camera) {
        _sprite.set_position(
            camera.to_screen_x(world_x),
            camera.to_screen_y(world_y)
        );
    }

    bn::sprite_ptr& sprite() { return _sprite; }

private:
    bn::sprite_ptr _sprite;
};