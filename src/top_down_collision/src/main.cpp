/*
 * Copyright (c) 2020-2026 Gustavo Valiente gustavo.valiente@protonmail.com
 * zlib License, see LICENSE file.
 */

#include "bn_core.h"
#include "bn_keypad.h"
#include "bn_optional.h"
#include "bn_point.h"
#include "bn_fixed.h"
#include "bn_vector.h"
#include "bn_string_view.h"
#include "bn_regular_bg_ptr.h"
#include "bn_sprite_ptr.h"
#include "bn_sprite_text_generator.h"
#include "bn_regular_bg_map_cell_info.h"
#include "bn_bg_palettes.h"
#include "bn_sprite_palettes.h"
#include "bn_color.h"

#include "bn_sprite_items_dog.h"
#include "bn_sprite_items_chest.h"
#include "bn_regular_bg_items_map.h"

#include "common_info.h"
#include "common_variable_8x16_sprite_font.h"

namespace
{
    enum class scene_type
    {
        TITLE,
        GAME
    };

    constexpr int text_sprites_capacity = 64;

    // Only at this tile, and only when moving right into the wall,
    // the map transition is triggered.
    constexpr bn::point map_exit_position(16, 11);
    constexpr bn::point map_exit_direction(0, -1);

    void show_title_screen(bn::sprite_text_generator& text_generator,
                           bn::vector<bn::sprite_ptr, text_sprites_capacity>& text_sprites)
    {
        text_sprites.clear();
        text_generator.generate(0, -24, "MAP COLLISION", text_sprites);
        text_generator.generate(0, 0, "PRESS START", text_sprites);
    }

    void show_game_ui(bn::sprite_text_generator& text_generator,
                    bn::vector<bn::sprite_ptr, text_sprites_capacity>& text_sprites)
    {
        text_sprites.clear();
        text_generator.generate(-50, -74, "PAD: MOVE DOG", text_sprites);
        text_generator.generate(-50, -62, "SPECIAL WALL = NEXT MAP", text_sprites);
        text_generator.generate(-50, -50, "START: TITLE", text_sprites);
    }

    // Resets the current map and the object positions.
    void load_map(bn::regular_bg_ptr& map_bg,
                  bn::point& dog_map_position,
                  bn::point& chest_map_position)
    {
        map_bg = bn::regular_bg_items::map.create_bg(0, 0);
        dog_map_position = bn::point(16, 16);
        chest_map_position = bn::point(18, 16);
    }

    // Returns true only for the one wall that should trigger a map change.
    bool is_map_exit(const bn::point& current_position, const bn::point& direction)
    {
        return current_position == map_exit_position && direction == map_exit_direction;
    }

    void fade_out()
    {
        for(int intensity = 0; intensity <= 16; ++intensity)
        {
            bn::fixed fade = intensity / 16.0;
            bn::bg_palettes::set_fade(bn::color(0, 0, 0), fade);
            bn::sprite_palettes::set_fade(bn::color(0, 0, 0), fade);
            bn::core::update();
        }
    }

    void fade_in()
    {
        for(int intensity = 16; intensity >= 0; --intensity)
        {
            bn::fixed fade = intensity / 16.0;
            bn::bg_palettes::set_fade(bn::color(0, 0, 0), fade);
            bn::sprite_palettes::set_fade(bn::color(0, 0, 0), fade);
            bn::core::update();
        }
    }
}

int main()
{
    bn::core::init();

    bn::sprite_text_generator text_generator(common::variable_8x16_sprite_font);
    bn::vector<bn::sprite_ptr, text_sprites_capacity> text_sprites;

    scene_type current_scene = scene_type::TITLE;

    bn::optional<bn::regular_bg_ptr> map_bg;
    bn::optional<bn::sprite_ptr> dog_sprite;
    bn::optional<bn::sprite_ptr> chest_sprite;

    // Positions are stored in map tiles, not in pixels.
    bn::point dog_map_position(16, 16);
    bn::point chest_map_position(18, 16);

    show_title_screen(text_generator, text_sprites);

    chest_map_position = bn::point(dog_map_position.x() + 2, dog_map_position.y());

    while (true)
    {
        if(current_scene == scene_type::TITLE)
        {
            if(bn::keypad::start_pressed())
            {
                fade_out();

                text_sprites.clear();

                map_bg = bn::regular_bg_items::map.create_bg(0, 0);
                dog_sprite = bn::sprite_items::dog.create_sprite(0, 0);
                chest_sprite = bn::sprite_items::chest.create_sprite(0, 0);

                dog_map_position = bn::point(16, 16);
                chest_map_position = bn::point(18, 16);

                show_game_ui(text_generator, text_sprites);
                current_scene = scene_type::GAME;

                fade_in();
            }
        }
        else if(current_scene == scene_type::GAME)
        {
            if(bn::keypad::start_pressed())
            {
                fade_out();

                chest_sprite.reset();
                dog_sprite.reset();
                map_bg.reset();

                show_title_screen(text_generator, text_sprites);
                current_scene = scene_type::TITLE;

                fade_in();
            }
            else
            {
                const bn::regular_bg_map_item& map_item = bn::regular_bg_items::map.map_item();

                // Tile (0, 0) is used here as the reference for a walkable tile.
                bn::regular_bg_map_cell valid_map_cell = map_item.cell(0, 0);
                int valid_tile_index = bn::regular_bg_map_cell_info(valid_map_cell).tile_index();

                bn::point new_dog_map_position = dog_map_position;

                if(bn::keypad::left_pressed())
                {
                    new_dog_map_position.set_x(new_dog_map_position.x() - 1);
                    dog_sprite->set_horizontal_flip(true);
                }
                else if(bn::keypad::right_pressed())
                {
                    new_dog_map_position.set_x(new_dog_map_position.x() + 1);
                    dog_sprite->set_horizontal_flip(false);
                }

                if(bn::keypad::up_pressed())
                {
                    new_dog_map_position.set_y(new_dog_map_position.y() - 1);
                }
                else if(bn::keypad::down_pressed())
                {
                    new_dog_map_position.set_y(new_dog_map_position.y() + 1);
                }

                bn::point direction = new_dog_map_position - dog_map_position;

                // Check the tile the dog wants to enter.
                bn::regular_bg_map_cell dog_map_cell = map_item.cell(new_dog_map_position);
                int dog_tile_index = bn::regular_bg_map_cell_info(dog_map_cell).tile_index();

                if(dog_tile_index == valid_tile_index)
                {
                    // If the dog moves into the chest, try to push it.
                    if(new_dog_map_position == chest_map_position)
                    {
                        bn::point new_chest_map_position = chest_map_position + direction;

                        bn::regular_bg_map_cell chest_next_cell = map_item.cell(new_chest_map_position);
                        int chest_next_tile_index =
                                bn::regular_bg_map_cell_info(chest_next_cell).tile_index();

                        if(chest_next_tile_index == valid_tile_index)
                        {
                            chest_map_position = new_chest_map_position;
                            dog_map_position = new_dog_map_position;
                        }
                    }
                    else
                    {
                        dog_map_position = new_dog_map_position;
                    }
                }
                else
                {
                    // Only this special wall triggers the map transition.
                    if(is_map_exit(dog_map_position, direction))
                    {
                        fade_out();
                        load_map(*map_bg, dog_map_position, chest_map_position);
                        fade_in();
                    }

                    // All other walls simply block movement.
                }

                // Convert tile coordinates to screen coordinates.
                bn::fixed dog_sprite_x =
                        (dog_map_position.x() * 8) - (map_item.dimensions().width() * 4) + 4;
                bn::fixed dog_sprite_y =
                        (dog_map_position.y() * 8) - (map_item.dimensions().height() * 4) + 4;
                dog_sprite->set_position(dog_sprite_x, dog_sprite_y);

                bn::fixed chest_sprite_x =
                        (chest_map_position.x() * 8) - (map_item.dimensions().width() * 4) + 4;
                bn::fixed chest_sprite_y =
                        (chest_map_position.y() * 8) - (map_item.dimensions().height() * 4) + 4;
                chest_sprite->set_position(chest_sprite_x, chest_sprite_y);

                // Z-Order refresh: we set the Z-Order based on the Y position of the sprites.
                dog_sprite->set_z_order(-dog_sprite_y.integer());
                chest_sprite->set_z_order(-chest_sprite_y.integer());
            }
        }

        bn::core::update();
    }
}