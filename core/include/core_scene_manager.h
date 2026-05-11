#pragma once

#include "bn_unique_ptr.h"
#include "core_scene.h"

namespace core {
    class SceneManager {
    public:
        static SceneManager& instance();
        
        SceneManager(const SceneManager&) = delete;
        void operator=(const SceneManager&) = delete;

        void set_next_scene(bn::unique_ptr<Scene> next_scene);
        void update();

    private:
        SceneManager() = default; // Konstruktor privat für Singleton
        bn::unique_ptr<Scene> _current_scene;
        bn::unique_ptr<Scene> _next_scene;
    };
}