#pragma once

#include "bn_array.h"
#include "bn_keypad.h"
#include "bn_optional.h"
#include "bn_sprite_ptr.h"
#include "bn_sprite_text_generator.h"
#include "bn_string.h"
#include "bn_string_view.h"
#include "bn_vector.h"

namespace engine {
namespace battle {

template<int ChatboxSpritesCapacity = 96, int MenuSpritesCapacity = 96>
class BattleChatboxMenu
{
public:
    static constexpr int option_count = 4;
    static constexpr int menu_column_width = 14;

    explicit BattleChatboxMenu(bn::sprite_text_generator& text_generator) :
        _text_generator(text_generator)
    {
        _option_labels[0] = "FIGHT";
        _option_labels[1] = "BAG";
        _option_labels[2] = "POKEMON";
        _option_labels[3] = "RUN";
        _message = "Choose an action.";

        _redraw_chatbox();
        _redraw_menu();
    }

    void set_message(bn::string_view message)
    {
        _assign_text(message, _message);
        _redraw_chatbox();
    }

    bool set_option_label(int option_index, bn::string_view label)
    {
        if(option_index < 0 || option_index >= option_count)
        {
            return false;
        }

        _assign_text(label, _option_labels[option_index]);
        _redraw_menu();
        return true;
    }

    [[nodiscard]] int selected_index() const
    {
        return _selected_index;
    }

    [[nodiscard]] bn::optional<int> update()
    {
        bool selection_changed = false;

        if(bn::keypad::left_pressed())
        {
            _move_selection(0, -1);
            selection_changed = true;
        }
        else if(bn::keypad::right_pressed())
        {
            _move_selection(0, 1);
            selection_changed = true;
        }
        else if(bn::keypad::up_pressed())
        {
            _move_selection(-1, 0);
            selection_changed = true;
        }
        else if(bn::keypad::down_pressed())
        {
            _move_selection(1, 0);
            selection_changed = true;
        }

        if(selection_changed)
        {
            _redraw_menu();
        }

        if(bn::keypad::a_pressed())
        {
            return _selected_index;
        }

        return bn::nullopt;
    }

private:
    bn::sprite_text_generator& _text_generator;
    bn::vector<bn::sprite_ptr, ChatboxSpritesCapacity> _chatbox_sprites;
    bn::vector<bn::sprite_ptr, MenuSpritesCapacity> _menu_sprites;
    bn::array<bn::string<10>, option_count> _option_labels;
    bn::string<28> _message;
    int _selected_index = 0;

    static void _assign_text(bn::string_view source, bn::istring& destination)
    {
        destination.clear();

        for(char character : source)
        {
            if(destination.full())
            {
                break;
            }

            destination.push_back(character);
        }
    }

    void _move_selection(int row_delta, int column_delta)
    {
        int row = _selected_index / 2;
        int column = _selected_index % 2;

        row = (row + row_delta + 2) % 2;
        column = (column + column_delta + 2) % 2;
        _selected_index = row * 2 + column;
    }

    static void _append_option(const bn::string<10>& label, bool selected, bn::string<28>& line)
    {
        line.append(selected ? "> " : "  ");
        line.append(label);

        while(line.size() < menu_column_width)
        {
            line.push_back(' ');
        }
    }

    void _redraw_chatbox()
    {
        _chatbox_sprites.clear();
        _text_generator.generate(-112, 20, "----------------------------", _chatbox_sprites);
        _text_generator.generate(-112, 34, _message.c_str(), _chatbox_sprites);
        _text_generator.generate(-112, 48, "----------------------------", _chatbox_sprites);
    }

    void _redraw_menu()
    {
        _menu_sprites.clear();

        bn::string<28> top_line;
        _append_option(_option_labels[0], _selected_index == 0, top_line);
        _append_option(_option_labels[1], _selected_index == 1, top_line);

        bn::string<28> bottom_line;
        _append_option(_option_labels[2], _selected_index == 2, bottom_line);
        _append_option(_option_labels[3], _selected_index == 3, bottom_line);

        _text_generator.generate(-112, 62, top_line.c_str(), _menu_sprites);
        _text_generator.generate(-112, 74, bottom_line.c_str(), _menu_sprites);
    }
};

} // namespace battle
} // namespace engine
