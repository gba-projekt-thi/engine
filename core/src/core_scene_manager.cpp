#include "core_scene_manager.h"
#include "bn_blending.h"
#include "bn_colors.h"
#include "bn_utility.h"

namespace core {
    SceneManager& SceneManager::instance() {
        static SceneManager manager;
        return manager;
    }

    void SceneManager::set_next_scene(bn::unique_ptr<Scene> next_scene)
    {
        if (!_next_scene)
        { // Only if a transition is not already in progress
            _next_scene = bn::move(next_scene);
            // We do NOT switch _current_scene here; update() handles that.
        }
    }

void SceneManager::update() {
    switch(_state) {
        case State::IDLE:
            if(_next_scene) {
                _state = State::FADE_OUT;
                _fade_counter = 0;
            }
            if(_current_scene) _current_scene->update();
            break;

        case State::FADE_OUT:
            _fade_counter++;
            // Calculate alpha value:
            bn::blending::set_fade_alpha(bn::fixed(_fade_counter) / FADE_FRAMES);
            
            // The old scene is allowed to update one last time (animations keep running)
            if(_current_scene) _current_scene->update();

            if(_fade_counter >= FADE_FRAMES) {
                _state = State::SWAPPING;
            }
            break;

        case State::SWAPPING:
            // --- THE CRITICAL MOMENT (RAII + INIT) ---
            _current_scene.reset();                 // 1. clear VRAM
            _current_scene = bn::move(_next_scene); // 2. take ownership
            
            if(_current_scene) {
                _current_scene->init();             // 3. load new assets
            }
            
            _state = State::FADE_IN;
            break;

        case State::FADE_IN:
            _fade_counter--;
            bn::blending::set_fade_alpha(bn::fixed(_fade_counter) / FADE_FRAMES);
            
            // The new scene is already running behind the fade!
            if(_current_scene) _current_scene->update();

            if(_fade_counter <= 0) {
                _state = State::IDLE;
                bn::blending::set_fade_alpha(0); // Ensure everything is fully visible
            }
            break;

        default:
            break;
    }
}

} // namespace core
