#include "bn_core.h"
#include "bn_keypad.h"
#include "bn_fixed.h"
#include "bn_size.h"
#include "bn_regular_bg_ptr.h"
#include "bn_regular_bg_item.h"
#include "bn_regular_bg_map_item.h"
#include "bn_regular_bg_tiles_items_simple_tileset.h"
#include "bn_bg_palette_items_palette.h"
#include "bn_sprite_items_frog.h"
#include "bn_sprite_items_enemy.h"
#include "bn_sprite_items_door.h"
#include "bn_sprite_items_laser.h"
#include "bn_sprite_text_generator.h"
#include "bn_vector.h"
#include "bn_string.h"
#include "common_variable_8x8_sprite_font.h"

#include "collision_registry.h"
#include "collision_config.h"
#include "tilemap_data.h"
#include "camera.h"
#include "sprite.h"
#include "sprite_registry.h"
#include "player.h"
#include "enemy.h"

// MASK_* and TYPE_* live in collision_config.h

int main() {
    bn::core::init();

    // --- BG ---
    bn::regular_bg_map_item map_item(map_cells[0], bn::size(64, 64));
    bn::regular_bg_item bg_item(
        bn::regular_bg_tiles_items::simple_tileset,
        bn::bg_palette_items::palette,
        map_item);
    bn::regular_bg_ptr bg = bg_item.create_bg(0, 0);

    // --- Map collision rects ---
    CollisionRegistry::instance().load_map_rects(map_rects, map_rect_count);

    // --- Camera ---
    Camera::instance().init(map_width * 8, map_height * 8);

    // --- Player ---
    bn::fixed start_x = 10 * 8;    // tile col 10 → 80 px
    bn::fixed start_y = 53 * 8;    // tile row 53 → 424 px

    Sprite frog_sprite(bn::sprite_items::frog.create_sprite(0, 0), start_x, start_y);

    Player player(start_x, start_y, 16, 16);   // collision box (tunable, smaller than 32x32 sprite)
    player.sprite = &frog_sprite;
    frog_sprite.pos.offset_y = -7;

    // --- Door ---
    bn::fixed door_x = 9 * 8;
    bn::fixed door_y = 16 * 8 + 4;

    Sprite door_sprite(bn::sprite_items::door.create_sprite(0, 0), door_x, door_y);
    door_sprite.sprite().set_z_order(1);

    StaticBody door(door_x, door_y, 16, 25, MASK_DOOR);
    door.sprite = &door_sprite;

    // --- Enemy ---
    bn::fixed enemy_x1 = 72;       // patrol left  (tile 9  * 8)
    bn::fixed enemy_x2 = 200;      // patrol right (tile 25 * 8)
    bn::fixed enemy_y  = 336;      // tile row 42 * 8

    Sprite enemy_sprite(bn::sprite_items::enemy.create_sprite(0, 0), enemy_x1, enemy_y);

    Enemy enemy(enemy_x1, enemy_y, 16, 24, enemy_x1, enemy_x2, bn::fixed(0.5));
    enemy.sprite = &enemy_sprite;

    // --- Laser beams ---
    Sprite laser_left_sprite(bn::sprite_items::laser.create_sprite(0, 0), enemy_x1, enemy_y);
    laser_left_sprite.pos.offset_x = -Enemy::LASER_OFFSET_X;
    laser_left_sprite.disable();

    Sprite laser_right_sprite(bn::sprite_items::laser.create_sprite(0, 0), enemy_x1, enemy_y);
    laser_right_sprite.pos.offset_x = Enemy::LASER_OFFSET_X;
    laser_right_sprite.disable();

    enemy.laser_left.beam  = &laser_left_sprite;
    enemy.laser_right.beam = &laser_right_sprite;

    // --- Debug text ---
    bn::sprite_text_generator text_gen(common::variable_8x8_sprite_font);
    bn::vector<bn::sprite_ptr, 32> text_sprites;

    // --- Main loop ---
    while(true) {
        // 1. physics: update() then move_velocity() for all PhysicsBodies
        CollisionRegistry::instance().update_all();

        // 2. camera follows player in world space
        Camera::instance().follow(player.pos.x, player.pos.y);

        // 3. BG scrolls
        bg.set_position(Camera::instance().bg_x(), Camera::instance().bg_y());

        // 4. all sprites: world → screen
        SpriteRegistry::instance().sync_all(Camera::instance());

        // 5. debug text
        text_sprites.clear();
        bn::string<32> info = bn::to_string<16>(player.pos.x.integer());
        info.append(", ");
        info.append(bn::to_string<16>(player.pos.y.integer()));
        text_gen.generate(-110, -70, info, text_sprites);

        // 6. flush to hardware
        bn::core::update();
    }
}