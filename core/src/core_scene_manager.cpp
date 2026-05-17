#include "core_scene_manager.h"
#include "bn_blending.h"
#include "bn_music.h"
#include "bn_colors.h"
#include "bn_utility.h"

namespace core
{
    SceneManager &SceneManager::instance()
    {
        static SceneManager manager;
        return manager;
    }

    void SceneManager::set_next_scene(bn::unique_ptr<Scene> next_scene, AudioTransitionOptions audio_opts)
    {
        if (!_next_scene)
        { // Only if a transition is not already in progress
            _next_scene = bn::move(next_scene);
            _audio_opts = audio_opts; // Remember the options for this transition.
            // We do NOT switch _current_scene here; update() handles that.
        }
    }

    void SceneManager::update()
    {
        switch (_state)
        {
        case State::IDLE:
            if (_next_scene)
            {
                _state = State::FADE_OUT;
                _fade_counter = 1; // Start directly at 1

                // AUDIO: If instant stop is requested, stop music immediately
                if (_audio_opts.stop_music_instantly)
                {
                    bn::music::stop();
                }

                bn::blending::set_fade_alpha(bn::fixed(_fade_counter) / FADE_FRAMES);
            }
            if (_current_scene)
                _current_scene->update();
            break;

        case State::FADE_OUT:
            if (_current_scene)
                _current_scene->update();

            // Increment first, then check
            _fade_counter++;
            {
                bn::fixed progress = bn::fixed(_fade_counter) / FADE_FRAMES;
                bn::blending::set_fade_alpha(progress);
                // AUDIO: Fade music linearly (only if instant stop is not active)
                if (_audio_opts.fade_music && !_audio_opts.stop_music_instantly)
                {
                    bn::music::set_volume(1.0 - progress);
                }
            }
            if (_fade_counter >= FADE_FRAMES)
            {
                // OPTIMIZATION: Do the swap directly here!
                _current_scene.reset();
                _current_scene = bn::move(_next_scene);
                if (_current_scene)
                    _current_scene->init();

                _state = State::FADE_IN;
                // The counter stays at MAX, so FADE_IN starts at MAX-1 in the next frame
            }
            break;

        case State::FADE_IN:
            _fade_counter--;
            {
                bn::fixed progress = bn::fixed(_fade_counter) / FADE_FRAMES;
                bn::blending::set_fade_alpha(progress);

                // AUDIO: Fade music in linearly
                if (_audio_opts.fade_music && !_audio_opts.stop_music_instantly)
                {
                    bn::music::set_volume(progress);
                }
            }

            if (_current_scene)
                _current_scene->update();

            if (_fade_counter <= 0)
            {
                _state = State::IDLE;
                bn::blending::set_fade_alpha(0);

                // AUDIO: Safety net — restore volume to full for the next scene
                bn::music::set_volume(1.0);
            }
            break;
        }
    }

} // namespace core
