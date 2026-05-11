#pragma once

namespace core {
    /**
     * @brief Basis-Interface für alle Spielzustände.
     * Jede Szene (Menü, RPG-Welt, Platformer-Level) erbt hiervon.
     */
    class Scene {
    public:
        virtual ~Scene() = default;

        // Wird beim Szenenwechsel einmalig nach dem Freigeben der alten Szene aufgerufen.
        virtual void init() {
        }
        
        // Wird jeden Frame vom SceneManager aufgerufen
        virtual void update() = 0;
    };
}