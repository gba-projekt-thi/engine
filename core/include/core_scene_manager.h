#pragma once

#include "bn_unique_ptr.h"
#include "bn_utility.h"
#include "core_scene.h"

namespace core {

    /**
     * @brief Manages the lifecycle of Scenes. 
     * Handles RAM cleanup automatically between transitions.
     */
    class SceneManager {
    public:
        static SceneManager& instance() {
            static SceneManager mgr;
            return mgr;
        }

        // Switches to a new scene at the start of the next frame
        void set_next_scene(bn::unique_ptr<Scene> next_scene) {
            _next_scene = bn::move(next_scene);
        }

        // Must be called in the main while(true) loop
        void update() {
            if(_next_scene) {
                // IMPORTANT: The old scene is destroyed HERE.
                // This clears all VRAM (Sprites/Maps) before the new one starts.
                _current_scene.reset(); 
                _current_scene = bn::move(_next_scene);
            }

            if(_current_scene) {
                _current_scene->update();
            }
        }

    private:
        SceneManager() = default;
        bn::unique_ptr<Scene> _current_scene;
        bn::unique_ptr<Scene> _next_scene;
    };

} // namespace core