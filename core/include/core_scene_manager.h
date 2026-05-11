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
        enum class State
        {
            IDLE,     // Normaler Spielverlauf
            FADE_OUT, // Wird schwarz
            SWAPPING, // RAII-Wechsel (VRAM-safe)
            FADE_IN   // Wird wieder sichtbar
        };

        State _state = State::IDLE;
        int _fade_counter = 0;
        static constexpr int FADE_FRAMES = 30; // 0.5 Sekunden bei 60 FPS

        SceneManager() = default; // Konstruktor privat für Singleton
        bn::unique_ptr<Scene> _current_scene;
        bn::unique_ptr<Scene> _next_scene;
    };
}