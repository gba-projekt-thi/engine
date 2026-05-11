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
        { // Nur wenn nicht gerade schon ein Wechsel läuft
            _next_scene = bn::move(next_scene);
            // Wir setzen NICHT _current_scene um, sondern überlassen das der update() Methode!
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
            // Berechnung des Alpha-Werts:
            bn::blending::set_fade_alpha(bn::fixed(_fade_counter) / FADE_FRAMES);
            
            // Die alte Szene darf noch ein letztes Mal updaten (Animationen laufen weiter)
            if(_current_scene) _current_scene->update();

            if(_fade_counter >= FADE_FRAMES) {
                _state = State::SWAPPING;
            }
            break;

        case State::SWAPPING:
            // --- DER KRITISCHE MOMENT (RAII + INIT) ---
            _current_scene.reset();                 // 1. VRAM leeren
            _current_scene = bn::move(_next_scene); // 2. Besitz übernehmen
            
            if(_current_scene) {
                _current_scene->init();             // 3. Neue Assets laden
            }
            
            _state = State::FADE_IN;
            break;

        case State::FADE_IN:
            _fade_counter--;
            bn::blending::set_fade_alpha(bn::fixed(_fade_counter) / FADE_FRAMES);
            
            // Die neue Szene läuft bereits im Hintergrund des Fades!
            if(_current_scene) _current_scene->update();

            if(_fade_counter <= 0) {
                _state = State::IDLE;
                bn::blending::set_fade_alpha(0); // Sicherstellen, dass alles sichtbar ist
            }
            break;

        default:
            break;
    }
}

} // namespace core
