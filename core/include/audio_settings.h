#pragma once

#include <cstdint>
#include "bn_fixed.h"
#include "bn_sound_item.h"

// -----------------------------------------------------------------------------
// AudioSettings
//
// Engine-level singleton holding the player's chosen music/SFX volume
// levels (0..MAX_LEVEL, default MAX_LEVEL = full volume). Generic and
// game-agnostic, like Camera/SpriteRegistry - lives in the engine layer so
// SceneManager can read music_scale() during fade transitions without the
// engine depending on any game-side type (dependency direction rule).
//
// Game code is responsible for persisting the levels into its own save
// data and restoring them into this singleton on load.
// -----------------------------------------------------------------------------
class AudioSettings {
   public:
    static constexpr uint8_t MAX_LEVEL = 4;

    static AudioSettings& instance() {
        static AudioSettings settings;
        return settings;
    }

    void set_music_level(uint8_t level) {
        _music_level = level > MAX_LEVEL ? MAX_LEVEL : level;
    }

    void set_sfx_level(uint8_t level) {
        _sfx_level = level > MAX_LEVEL ? MAX_LEVEL : level;
    }

    [[nodiscard]] uint8_t music_level() const { return _music_level; }
    [[nodiscard]] uint8_t sfx_level() const { return _sfx_level; }

    [[nodiscard]] bn::fixed music_scale() const {
        return bn::fixed(_music_level) / MAX_LEVEL;
    }

    [[nodiscard]] bn::fixed sfx_scale() const {
        return bn::fixed(_sfx_level) / MAX_LEVEL;
    }

    // Plays a one-shot sound effect scaled by the current SFX level. Use
    // instead of calling bn::sound_item::play() directly so new call
    // sites automatically respect the player's SFX setting. No-ops at
    // level 0 rather than calling play(0), avoiding a wasted channel.
    void play_sfx(bn::sound_item item) const {
        if (_sfx_level == 0) {
            return;
        }
        item.play(sfx_scale());
    }

   private:
    AudioSettings() = default;

    uint8_t _music_level = MAX_LEVEL;
    uint8_t _sfx_level = MAX_LEVEL;
};