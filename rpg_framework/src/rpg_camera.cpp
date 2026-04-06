#include "rpg_camera.h"
#include "bn_math.h"

namespace rpg {

    // Constructor: The internal Butano camera is created at (0,0)
    Camera::Camera() : _camera(bn::camera_ptr::create(0, 0)) {}

    void Camera::init(int map_w, int map_h) {
        _map_w = map_w;
        _map_h = map_h;
    }

    void Camera::follow(const bn::fixed_point& target_pos) {
        // Half of the GBA screen dimensions (240x160)
        bn::fixed half_width = 120;
        bn::fixed half_height = 80;

        // If the map is smaller than the screen, we just stay at the center
        // Otherwise, we clamp the focus point so the camera doesn't peek over the edges
        bn::fixed x_min = bn::min(half_width, bn::fixed(_map_w) / 2);
        bn::fixed x_max = bn::max(half_width, bn::fixed(_map_w) - half_width);
        
        bn::fixed y_min = bn::min(half_height, bn::fixed(_map_h) / 2);
        bn::fixed y_max = bn::max(half_height, bn::fixed(_map_h) - half_height);

        bn::fixed clamped_x = bn::clamp(target_pos.x(), x_min, x_max);
        bn::fixed clamped_y = bn::clamp(target_pos.y(), y_min, y_max);

        // Setting the camera position. 
        // We subtract the half-dimensions to align world (0,0) with screen (0,0)
        _camera.set_position(clamped_x - 120, clamped_y - 80);
    }

    bn::fixed_point Camera::to_screen(const bn::fixed_point& world_pos) const {
        // Standard transformation: ScreenPos = WorldPos - CameraPosition
        // This is essential for placing UI elements or effects 
        // relative to moving world objects.
        return bn::fixed_point(world_pos.x() - _camera.x(), 
                               world_pos.y() - _camera.y());
    }

} // namespace rpg