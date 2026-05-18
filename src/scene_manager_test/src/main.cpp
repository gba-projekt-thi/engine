#include "bn_core.h"
#include "bn_keypad.h"
#include "bn_log.h"
#include "bn_unique_ptr.h"
#include "bn_vector.h"
#include "bn_sprite_ptr.h"
#include "bn_sprite_text_generator.h"
#include "bn_bg_palettes.h"
#include "bn_color.h"
#include "bn_blending.h"
#include "bn_colors.h"
#include "bn_music_items.h"

#include "core_scene.h"
#include "core_scene_manager.h"

#include "common_variable_8x16_sprite_font.h"

// ─── Forward declarations ─────────────────────────────────────────────────────
class TitleScene;
class LevelScene;

// ─── Player ──────────────────────────────────────────────────────────────────
// Represents a game object whose lifetime is managed by the owning scene.
// BN_LOG in the destructor proves that nested RAII works correctly.
class Player
{
public:
    Player() { BN_LOG("CONSTRUCTOR: Player created."); }
    ~Player() { BN_LOG("DESTRUCTOR:  Player removed from memory."); }
};

// ─── TitleScene ───────────────────────────────────────────────────────────────
class TitleScene : public core::Scene
{
public:
    explicit TitleScene(bn::sprite_text_generator &gen);
    ~TitleScene() override;

    void init() override;
    void update() override;

private:
    bn::sprite_text_generator &_gen;
    bn::vector<bn::sprite_ptr, 64> _sprites;
};

// ─── LevelScene ───────────────────────────────────────────────────────────────
class LevelScene : public core::Scene
{
public:
    explicit LevelScene(bn::sprite_text_generator &gen);
    ~LevelScene() override;

    void init() override;
    void update() override;

private:
    bn::sprite_text_generator &_gen;
    Player _player; // nested RAII object
    bn::vector<bn::sprite_ptr, 64> _sprites;
};

// ─── TitleScene implementation ────────────────────────────────────────────────
TitleScene::TitleScene(bn::sprite_text_generator& gen) : _gen(gen)
{
    // The constructor is now almost empty. Only logging, no sprites!
    BN_LOG("CONSTRUCTOR: TitleScene (Lightweight)");
}

void TitleScene::init() 
{
    bn::music_items::game_audio_level1.play();
    // Only NOW, when the SceneManager says so, are the sprites created.
    BN_LOG("INIT: TitleScene - Creating Sprites now.");
    _gen.generate(4 - 120, -20, "=== TITLE SCENE ===", _sprites);
    _gen.generate(4 - 120,   0, "START -> LevelScene", _sprites);
}

TitleScene::~TitleScene()
{
    BN_LOG("DESTRUCTOR: TitleScene deleted.");
}

void TitleScene::update()
{
    if (bn::keypad::start_pressed())
    {
        core::SceneManager::instance().set_next_scene(
            bn::make_unique<LevelScene>(_gen));
    }
}

// ─── LevelScene implementation ────────────────────────────────────────────────
LevelScene::LevelScene(bn::sprite_text_generator& gen) : _gen(gen)
{
    BN_LOG("CONSTRUCTOR: LevelScene (Lightweight)");
}

void LevelScene::init() 
{
    bn::music_items::game_audio_level2.play();
    BN_LOG("INIT: LevelScene - Creating Sprites now.");
    _gen.generate(4 - 120, -20, "=== LEVEL SCENE ===", _sprites);
    _gen.generate(4 - 120,   0, "START -> TitleScene", _sprites);
}

LevelScene::~LevelScene()
{
    BN_LOG("DESTRUCTOR: LevelScene deleted.");
}

void LevelScene::update()
{
    if (bn::keypad::start_pressed())
    {
        core::SceneManager::instance().set_next_scene(
            bn::make_unique<TitleScene>(_gen));
    }
}

// ─── main ─────────────────────────────────────────────────────────────────────
// main.cpp is intentionally minimal: only the SceneManager lifecycle runs here.
// Verify RAII by opening Tools -> Message Log in mGBA and pressing START.
int main()
{
    bn::core::init();

    // 2. Blending configuration (once for the entire game)
    bn::blending::set_fade_color(bn::blending::fade_color_type::BLACK);
    bn::blending::set_fade_alpha(0); // Starts fully visible

    bn::sprite_text_generator gen(common::variable_8x16_sprite_font);

    gen.set_blending_enabled(true);

    bn::bg_palettes::set_transparent_color(bn::color(0, 0, 0));

    // The first scene is set (the SceneManager starts the process)
    core::SceneManager::instance().set_next_scene(bn::make_unique<TitleScene>(gen));

    while (true)
    {
        core::SceneManager::instance().update();
        bn::core::update();
    }
}
