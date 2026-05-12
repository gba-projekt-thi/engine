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
                _fade_counter = 1; // Start directly at 1
                bn::blending::set_fade_alpha(bn::fixed(_fade_counter) / FADE_FRAMES);
            }
            if(_current_scene) _current_scene->update();
            break;

        case State::FADE_OUT:
            if(_current_scene) _current_scene->update();
            
            // Increment first, then check
            _fade_counter++;
            bn::blending::set_fade_alpha(bn::fixed(_fade_counter) / FADE_FRAMES);

            if(_fade_counter >= FADE_FRAMES) {
                // OPTIMIZATION: Do the swap directly here!
                _current_scene.reset();
                _current_scene = bn::move(_next_scene);
                if(_current_scene) _current_scene->init();
                
                _state = State::FADE_IN;
                // The counter stays at MAX, so FADE_IN starts at MAX-1 in the next frame
            }
            break;

        case State::FADE_IN:
            _fade_counter--;
            bn::blending::set_fade_alpha(bn::fixed(_fade_counter) / FADE_FRAMES);
            
            if(_current_scene) _current_scene->update();

            if(_fade_counter <= 0) {
                _state = State::IDLE;
                bn::blending::set_fade_alpha(0);
            }
            break;
    }
}

} // namespace core
