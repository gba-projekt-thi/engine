#include "core_scene_manager.h"
#include "bn_blending.h"
#include "bn_colors.h"
#include "bn_utility.h"

namespace core {
    SceneManager& SceneManager::instance() {
        static SceneManager manager;
        return manager;
    }

    void SceneManager::set_next_scene(bn::unique_ptr<Scene> next_scene) {
        _next_scene = bn::move(next_scene);
    }

    void SceneManager::update() {
        if(_next_scene) {
            bn::blending::set_fade_color(bn::colors::black);
            bn::blending::set_fade_alpha(1.0);

            // WECHSEL: RAII löscht die alte Szene (Testament wird geschrieben)
            _current_scene.reset(); 
            _current_scene = bn::move(_next_scene);

            bn::blending::set_fade_alpha(0.0);
        }

        if(_current_scene) {
            _current_scene->update();
        }
    }
}
