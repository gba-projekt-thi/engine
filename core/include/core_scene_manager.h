#pragma once

#include "bn_unique_ptr.h"
#include "core_scene.h"

namespace core {

    // 1. Define the options (at the top of the namespace)
struct AudioTransitionOptions {
    bool fade_music = true;            // Music fades by default
    bool stop_music_instantly = false; // For immediate cut (e.g. pause)
};

    class SceneManager {
    public:
        static SceneManager& instance();
        
        SceneManager(const SceneManager&) = delete;
        void operator=(const SceneManager&) = delete;

        void set_next_scene(bn::unique_ptr<Scene> next_scene, AudioTransitionOptions audio_opts = {});
        void update();

    private:
        enum class State
        {
            IDLE,     // Normal gameplay
            FADE_OUT, // Fades to black
            FADE_IN   // Fades back in
        };

        State _state = State::IDLE;
        int _fade_counter = 0;
        static constexpr int FADE_FRAMES = 60; // 1 second at 60 FPS

        SceneManager() = default; // Private constructor for singleton
        bn::unique_ptr<Scene> _current_scene;
        bn::unique_ptr<Scene> _next_scene;

        AudioTransitionOptions _audio_opts;
    };
}