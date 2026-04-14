#include "rpg_camera.h"
#include "bn_math.h"

namespace rpg {

    // Constructor: The internal Butano camera is created at (0,0)
    Camera::Camera() : _camera(bn::camera_ptr::create(0, 0)) {}

    void Camera::init(int map_w, int map_h) {
        _map_w = map_w;
        _map_h = map_h;
    }

void Camera::follow(const bn::fixed_point& target_pos, bn::fixed lerp_speed) {
        // 1. Calculate the ideal target (where we WANT to be)
        // Same clamping logic as before to respect map boundaries
        bn::fixed target_x = bn::clamp(target_pos.x(), bn::fixed(120), bn::fixed(_map_w - 120));
        bn::fixed target_y = bn::clamp(target_pos.y(), bn::fixed(80), bn::fixed(_map_h - 80));

        // 2. Get the current camera position
        bn::fixed current_x = _camera.x();
        bn::fixed current_y = _camera.y();

        // 3. Apply Lerp (Linear Interpolation)
        // 0.1 means the camera covers 10% of the distance every frame.
        // Lower values (0.05) make it smoother/slower. 
        // Higher values (0.2) make it snappier.
        bn::fixed new_x = current_x + (target_x - current_x) * lerp_speed;
        bn::fixed new_y = current_y + (target_y - current_y) * lerp_speed;

        // 4. Update the actual camera pointer
        _camera.set_position(new_x, new_y);
    }

    bn::fixed_point Camera::to_screen(const bn::fixed_point& world_pos) const {
        // Standard transformation: ScreenPos = WorldPos - CameraPosition
        // This is essential for placing UI elements or effects 
        // relative to moving world objects.
        return bn::fixed_point(world_pos.x() - _camera.x(), 
                               world_pos.y() - _camera.y());
    }

} // namespace rpg