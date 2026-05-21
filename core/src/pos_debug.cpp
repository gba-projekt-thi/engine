#include "pos_debug.h"

#ifdef POS_DEBUG

#include "bn_optional.h"
#include "bn_sprite_ptr.h"
#include "bn_sprite_palette_ptr.h"
#include "bn_sprite_palette_item.h"
#include "bn_sprite_affine_mat_ptr.h"
#include "bn_sprite_double_size_mode.h"
#include "bn_sprite_items_debug_square_64.h"
#include "camera.h"
#include "collision_registry.h"

namespace pos_debug {

namespace {

    constexpr int MAX_ENTRIES  = 32;
    constexpr int MAX_PALETTES = 8;
    constexpr int MAX_AFFINES  = 16;
    constexpr bn::fixed BASE_SIZE = 64;

    struct Entry {
        const Position* pos = nullptr;
        bn::optional<bn::sprite_ptr> sprite;
    };

    struct StaticEntry {
        bn::fixed wx;
        bn::fixed wy;
        bn::optional<bn::sprite_ptr> sprite;
    };

    struct PaletteSlot {
        bn::color color;
        bn::optional<bn::sprite_palette_ptr> palette;
    };

    struct AffineSlot {
        bn::fixed w;
        bn::fixed h;
        bn::optional<bn::sprite_affine_mat_ptr> affine;
    };

    Entry       _entries[MAX_ENTRIES];
    int         _count = 0;
    StaticEntry _static_entries[MAX_ENTRIES];
    int         _static_count = 0;
    PaletteSlot _palettes[MAX_PALETTES];
    AffineSlot  _affines[MAX_AFFINES];

    bn::sprite_palette_ptr get_or_make_palette(bn::color color) {
        for(int i = 0; i < MAX_PALETTES; ++i) {
            if(_palettes[i].palette && _palettes[i].color == color) {
                return *_palettes[i].palette;
            }
        }
        for(int i = 0; i < MAX_PALETTES; ++i) {
            if(!_palettes[i].palette) {
                _palettes[i].color = color;
                alignas(4) bn::color colors[16] = {};
                colors[1] = color;
                bn::sprite_palette_item item(
                    bn::span<const bn::color>(colors, 16),
                    bn::bpp_mode::BPP_4);
                _palettes[i].palette = item.create_palette();
                return *_palettes[i].palette;
            }
        }
        // overflow: reuse slot 0
        return *_palettes[0].palette;
    }

    bn::sprite_affine_mat_ptr get_or_make_affine(bn::fixed w, bn::fixed h) {
        for(int i = 0; i < MAX_AFFINES; ++i) {
            if(_affines[i].affine && _affines[i].w == w && _affines[i].h == h) {
                return *_affines[i].affine;
            }
        }
        for(int i = 0; i < MAX_AFFINES; ++i) {
            if(!_affines[i].affine) {
                _affines[i].w = w;
                _affines[i].h = h;
                bn::sprite_affine_mat_ptr aff = bn::sprite_affine_mat_ptr::create();
                aff.set_horizontal_scale(w / BASE_SIZE);
                aff.set_vertical_scale(h / BASE_SIZE);
                _affines[i].affine = aff;
                return *_affines[i].affine;
            }
        }
        // overflow: reuse slot 0
        return *_affines[0].affine;
    }

    bn::sprite_ptr make_sprite(bn::fixed w, bn::fixed h, bn::color color) {
        bn::sprite_ptr spr = bn::sprite_items::debug_square_64.create_sprite(0, 0);
        spr.set_double_size_mode(bn::sprite_double_size_mode::ENABLED);
        spr.set_affine_mat(get_or_make_affine(w, h));
        spr.set_palette(get_or_make_palette(color));
        spr.set_bg_priority(0);
        spr.set_z_order(-32767);
        return spr;
    }

} // anonymous

void register_draw_position(const Position& pos, bn::color color) {
    if(_count >= MAX_ENTRIES) return;
    Entry& e = _entries[_count++];
    e.pos = &pos;
    e.sprite = make_sprite(pos.width, pos.height, color);
}

void register_draw_positions(const Position* positions, int count, bn::color color) {
    for(int i = 0; i < count; ++i) {
        register_draw_position(positions[i], color);
    }
}

void register_draw_rect(bn::fixed x_min, bn::fixed y_min,
                        bn::fixed x_max, bn::fixed y_max,
                        bn::color color) {
    if(_static_count >= MAX_ENTRIES) return;
    StaticEntry& e = _static_entries[_static_count++];
    e.wx = (x_min + x_max) / 2;
    e.wy = (y_min + y_max) / 2;
    e.sprite = make_sprite(x_max - x_min, y_max - y_min, color);
}

void register_draw_map_rects(bn::color color) {
    const MapRect* rects = CollisionRegistry::instance().get_map_rects();
    int count = CollisionRegistry::instance().get_map_rect_count();
    for(int i = 0; i < count; ++i) {
        register_draw_rect(rects[i].x_min, rects[i].y_min,
                           rects[i].x_max, rects[i].y_max, color);
    }
}

void update_all(const Camera& camera) {
    for(int i = 0; i < _count; ++i) {
        Entry& e = _entries[i];
        if(!e.pos || !e.sprite) continue;
        e.sprite->set_position(
            camera.to_screen_x(e.pos->get_x()),
            camera.to_screen_y(e.pos->get_y())
        );
    }
    for(int i = 0; i < _static_count; ++i) {
        StaticEntry& e = _static_entries[i];
        if(!e.sprite) continue;
        e.sprite->set_position(
            camera.to_screen_x(e.wx),
            camera.to_screen_y(e.wy)
        );
    }
}

void clear() {
    for(int i = 0; i < _count; ++i) {
        _entries[i].pos = nullptr;
        _entries[i].sprite.reset();
    }
    _count = 0;
    for(int i = 0; i < _static_count; ++i) {
        _static_entries[i].sprite.reset();
    }
    _static_count = 0;
}

} // namespace pos_debug

#endif // POS_DEBUG
