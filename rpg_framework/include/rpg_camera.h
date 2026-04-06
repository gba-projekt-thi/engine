#pragma once

#include "bn_camera_ptr.h"
#include "bn_fixed_point.h"

namespace rpg {

    /**
     * @brief Singleton class that manages the GBA camera and coordinate projection.
     */
    class Camera {
    public:
        /**
         * @brief Access the global Camera instance.
         */
        static Camera& instance() {
            static Camera _instance;
            return _instance;
        }

        /**
         * @brief Initialize camera boundaries based on map dimensions.
         * @param map_w Width of the map in pixels.
         * @param map_h Height of the map in pixels.
         */
        void init(int map_w, int map_h);

        /**
         * @brief Updates the camera to follow a specific world coordinate.
         * @param target_pos The current world position of the player/target.
         */
        void follow(const bn::fixed_point& target_pos);

        /**
         * @brief Converts a world-space coordinate to a GBA screen-space coordinate.
         * @param world_pos The position in the game world.
         * @return bn::fixed_point The projected position on the screen.
         */
        bn::fixed_point to_screen(const bn::fixed_point& world_pos) const;

        /**
         * @brief Returns the underlying Butano camera pointer.
         */
        bn::camera_ptr& ptr() { return _camera; }

    private:
        Camera(); // Private constructor for Singleton pattern
        bn::camera_ptr _camera;
        int _map_w = 0;
        int _map_h = 0;
    };

} // namespace rpg