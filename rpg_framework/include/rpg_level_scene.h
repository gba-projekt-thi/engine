#pragma once

#include "core_scene.h"
#include "rpg_camera.h"
#include "bn_fixed_point.h"
#include "bn_regular_bg_ptr.h"

namespace rpg
{

    /**
     * @brief Base class for all RPG levels.
     * Implements the Template Method pattern to ensure camera updates
     * always happen after game logic.
     */
    class LevelScene : public core::Scene
    {
    public:
        /**
         * @param map_w Map width in pixels.
         * @param map_h Map height in pixels.
         */
        LevelScene(int map_w, int map_h)
        {
            // Initialize the camera with map boundaries
            rpg::Camera::instance().init(map_w, map_h);
        }

        // Destructor is virtual and defaulted.
        // RAII ensures that all member variables (BGs, Sprites)
        // are cleared from VRAM when the scene is destroyed.
        virtual ~LevelScene() = default;

        /**
         * @brief Final implementation of the core update loop.
         * Orchestrates logic execution and camera following.
         */
        void update() final
        {
            // 1. Execute the specific level logic (movement, triggers, etc.)
            update_logic();

            // 2. Automatically update camera position based on focus point
            rpg::Camera::instance().follow(_camera_focus);
        }

        /**
         * @brief Abstract method for level-specific logic.
         * Must be implemented by child classes (e.g., ForestScene).
         */
        virtual void update_logic() = 0;

    protected:
        // The point the camera will try to follow (usually the player's position)
        bn::fixed_point _camera_focus;

        /**
         * @brief Utility to link a background to the RPG camera system.
         */
        void setup_background(bn::regular_bg_ptr &bg)
        {
            bg.set_camera(rpg::Camera::instance().ptr());
        }
        
        /**
         * @brief Links a sprite to the RPG camera.
         */
        void setup_sprite(bn::sprite_ptr &sprite)
        {
            sprite.set_camera(rpg::Camera::instance().ptr());
        }
    };

}