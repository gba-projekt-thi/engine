/*
 * Copyright (c) 2020-2025 Gustavo Valiente gustavo.valiente@protonmail.com
 * zlib License, see LICENSE file.
 */

#include "bn_core.h"
#include "bn_keypad.h"
#include "bn_fixed.h"
#include "bn_fixed_point.h"
#include "bn_point.h"
#include "bn_regular_bg_ptr.h"
#include "bn_sprite_ptr.h"
#include "bn_sprite_text_generator.h"
#include "bn_regular_bg_map_cell_info.h"

#include "bn_sprite_items_dog.h"
#include "bn_regular_bg_items_map.h"

#include "common_info.h"
#include "common_variable_8x16_sprite_font.h"

int main()
{
    bn::core::init();

    constexpr bn::string_view info_text_lines[] = {
        "PAD: move dog"};

    bn::sprite_text_generator text_generator(common::variable_8x16_sprite_font);
    common::info info("Map collision", info_text_lines, text_generator);

    bn::regular_bg_ptr map_bg = bn::regular_bg_items::map.create_bg(0, 0);
    bn::sprite_ptr dog_sprite = bn::sprite_items::dog.create_sprite(0, 0);

    const bn::regular_bg_map_item &map_item = bn::regular_bg_items::map.map_item();

    // Use the tile at (0, 0) as the walkable tile reference.
    bn::regular_bg_map_cell valid_map_cell = map_item.cell(0, 0);
    int valid_tile_index = bn::regular_bg_map_cell_info(valid_map_cell).tile_index();

    const int map_width_cells = map_item.dimensions().width();
    const int map_height_cells = map_item.dimensions().height();
    const int map_width_pixels = map_width_cells * 8;
    const int map_height_pixels = map_height_cells * 8;

    // Store the dog position in pixels, not tiles.
    bn::fixed_point dog_world_position((16 * 8) + 4, (16 * 8) + 4);

    constexpr bn::fixed move_speed = 0.75;
    constexpr bn::fixed diagonal_move_speed = 0.53;

    // Small hitbox around the dog's center.
    constexpr int hitbox_half_width = 3;
    constexpr int hitbox_half_height = 3;

    auto is_walkable_pixel = [&](bn::fixed world_x, bn::fixed world_y) -> bool
    {
        // Convert pixel position to tile position.
        int cell_x = (world_x / 8).floor_integer();
        int cell_y = (world_y / 8).floor_integer();

        if (cell_x < 0 || cell_x >= map_width_cells || cell_y < 0 || cell_y >= map_height_cells)
        {
            return false;
        }

        bn::regular_bg_map_cell cell = map_item.cell(cell_x, cell_y);
        int tile_index = bn::regular_bg_map_cell_info(cell).tile_index();
        return tile_index == valid_tile_index;
    };

    auto can_move_to = [&](const bn::fixed_point &position) -> bool
    {
        // Check the four corners of the hitbox.
        return is_walkable_pixel(position.x() - hitbox_half_width, position.y() - hitbox_half_height) &&
               is_walkable_pixel(position.x() + hitbox_half_width, position.y() - hitbox_half_height) &&
               is_walkable_pixel(position.x() - hitbox_half_width, position.y() + hitbox_half_height) &&
               is_walkable_pixel(position.x() + hitbox_half_width, position.y() + hitbox_half_height);
    };

    while (true)
    {
        bn::fixed move_x = 0;
        bn::fixed move_y = 0;

        if (bn::keypad::left_held())
        {
            move_x -= 1;
            dog_sprite.set_horizontal_flip(true);
        }
        else if (bn::keypad::right_held())
        {
            move_x += 1;
            dog_sprite.set_horizontal_flip(false);
        }

        if (bn::keypad::up_held())
        {
            move_y -= 1;
        }
        else if (bn::keypad::down_held())
        {
            move_y += 1;
        }

        if (move_x != 0 || move_y != 0)
        {
            // Reduce diagonal speed so movement stays consistent.
            bn::fixed applied_speed = (move_x != 0 && move_y != 0) ? diagonal_move_speed : move_speed;

            // Move on X first.
            bn::fixed_point next_position = dog_world_position;
            next_position.set_x(next_position.x() + move_x * applied_speed);

            if (can_move_to(next_position))
            {
                dog_world_position.set_x(next_position.x());
            }

            // Move on Y second for smoother wall sliding.
            next_position = dog_world_position;
            next_position.set_y(next_position.y() + move_y * applied_speed);

            if (can_move_to(next_position))
            {
                dog_world_position.set_y(next_position.y());
            }
        }

        // Convert world position to screen position.
        bn::fixed dog_sprite_x = dog_world_position.x() - (map_width_pixels / 2);
        bn::fixed dog_sprite_y = dog_world_position.y() - (map_height_pixels / 2);
        dog_sprite.set_position(dog_sprite_x, dog_sprite_y);

        info.update();
        bn::core::update();
    }
}