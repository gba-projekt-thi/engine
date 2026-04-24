#pragma once

#include "camera.h"

class Sprite;

class SpriteRegistry {

public:
    static SpriteRegistry& instance() {
        static SpriteRegistry reg;
        return reg;
    }

    void register_sprite(Sprite* s) {
        if(_count < MAX_SPRITES) {
            _sprites[_count] = s;
            ++_count;
        }
    }

    void unregister_sprite(Sprite* s) {
        for(int i = 0; i < _count; ++i) {
            if(_sprites[i] == s) {
                // swap with last, shrink
                _sprites[i] = _sprites[_count - 1];
                --_count;
                return;
            }
        }
    }

    void sync_all(const Camera& camera);

private:
    SpriteRegistry() = default;

    static constexpr int MAX_SPRITES = 64;
    Sprite* _sprites[MAX_SPRITES] = {};
    int _count = 0;
};