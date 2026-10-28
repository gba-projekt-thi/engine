#pragma once

#include "bn_sprite_ptr.h"
#include "bn_fixed.h"
#include "bn_size.h"
#include "bn_utility.h"
#include "camera.h"
#include "sprite_registry.h"
#include "position.h"

class Sprite {

public:
    Position pos;

    Sprite(bn::sprite_ptr spr, bn::fixed wx, bn::fixed wy)
        : pos{wx, wy, spr.dimensions().width(), spr.dimensions().height()},
          _sprite(bn::move(spr)) {
        SpriteRegistry::instance().register_sprite(this);
    }

    // Non-copyable, non-movable: SpriteRegistry stores a raw `this` pointer,
    // so a Sprite must never change address after construction.
    Sprite(const Sprite&) = delete;
    Sprite& operator=(const Sprite&) = delete;
    Sprite(Sprite&&) = delete;
    Sprite& operator=(Sprite&&) = delete;

    ~Sprite() {
        SpriteRegistry::instance().unregister_sprite(this);
    }

    void enable() {
        _enabled = true;
        _sprite.set_visible(true);
    }

    void disable() {
        _enabled = false;
        _sprite.set_visible(false);
    }

    bool is_enabled() const { return _enabled; }

    void sync(const Camera& camera) {
        _sprite.set_position(
            camera.to_screen_x(pos.get_x()),
            camera.to_screen_y(pos.get_y())
        );
    }

    bn::sprite_ptr& sprite() { return _sprite; }

private:
    bn::sprite_ptr _sprite;
    bool _enabled = true;
};