#pragma once

namespace core {
    /**
     * @brief Basis-Interface für alle Spielzustände.
     * Jede Szene (Menü, RPG-Welt, Platformer-Level) erbt hiervon.
     */
    class Scene {
    public:
        virtual ~Scene() = default;
        
        // Wird jeden Frame vom SceneManager aufgerufen
        virtual void update() = 0;
    };
}