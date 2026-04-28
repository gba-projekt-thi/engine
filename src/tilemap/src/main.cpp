#include "bn_core.h"
#include "bn_keypad.h"
#include "bn_fixed.h"
#include "bn_size.h"
#include "bn_regular_bg_ptr.h"
#include "bn_regular_bg_item.h"
#include "bn_regular_bg_map_item.h"
#include "bn_regular_bg_tiles_items_simple_tileset.h"
#include "bn_bg_palette_items_palette.h"
#include "simple_tilemap_data.h"

int main() {
    bn::core::init();

    bn::regular_bg_map_item map_item(map_cells[0], bn::size(map_width, map_height));
    bn::regular_bg_item bg_item(
        bn::regular_bg_tiles_items::simple_tileset,
        bn::bg_palette_items::palette,
        map_item);
    bn::regular_bg_ptr bg = bg_item.create_bg(0, 0);

    bn::fixed cam_x = 0;
    bn::fixed cam_y = 0;
    bn::fixed speed = 1;

    while(true) {
        if(bn::keypad::left_held()) cam_x += speed;
        if(bn::keypad::right_held()) cam_x -= speed;
        if(bn::keypad::up_held()) cam_y += speed;
        if(bn::keypad::down_held()) cam_y -= speed;

        bg.set_position(cam_x, cam_y);
        bn::core::update();
    }
}