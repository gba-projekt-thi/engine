#include "bn_core.h"
#include "bn_optional.h"
#include "bn_sprite_text_generator.h"

#include "battle_chatbox.h"
#include "common_variable_8x16_sprite_font.h"

namespace
{
    void update_message_from_choice(engine::battle::BattleChatboxMenu<>& menu, int selected_option)
    {
        switch(selected_option)
        {
        case 0:
            menu.set_message("You chose FIGHT!");
            break;
        case 1:
            menu.set_message("You opened the BAG!");
            break;
        case 2:
            menu.set_message("Select a POKEMON!");
            break;
        default:
            menu.set_message("You tried to RUN!");
            break;
        }
    }
}

int main()
{
    bn::core::init();

    bn::sprite_text_generator text_generator(common::variable_8x16_sprite_font);
    engine::battle::BattleChatboxMenu<> battle_menu(text_generator);

    battle_menu.set_message("A wild MON appeared!");
    battle_menu.set_option_label(0, "SKILL");
    battle_menu.set_option_label(1, "ITEM");
    battle_menu.set_option_label(2, "PARTY");
    battle_menu.set_option_label(3, "FLEE");

    while(true)
    {
        if(bn::optional<int> selected_option = battle_menu.update())
        {
            update_message_from_choice(battle_menu, *selected_option);
        }

        bn::core::update();
    }
}
