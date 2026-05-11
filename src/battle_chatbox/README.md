# Battle Chatbox System

Reusable Pokémon-style battle chatbox/menu UI for Butano games.

## Features

- Chatbox plus 2x2 battle menu (`FIGHT`, `BAG`, `POKEMON`, `RUN` by default)
- D-pad navigation between menu options
- `A` button selection callback (`selected_index`)
- Per-option label customization
- UI component separated from game battle logic

## Usage

```cpp
#include "battle_chatbox.h"

bn::sprite_text_generator text_generator(common::variable_8x16_sprite_font);
engine::battle::BattleChatboxMenu<> battle_menu(text_generator);

battle_menu.set_message("A wild MON appeared!");
battle_menu.set_option_label(0, "SKILL");
battle_menu.set_option_label(1, "ITEM");
battle_menu.set_option_label(2, "PARTY");
battle_menu.set_option_label(3, "FLEE");

while(true)
{
    if(bn::optional<int> choice = battle_menu.update())
    {
        // Handle selected action in your battle logic
    }

    bn::core::update();
}
```

## Demo

See `src/battle_chatbox_test` for a simple playable scene.