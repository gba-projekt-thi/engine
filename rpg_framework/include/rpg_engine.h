#pragma once

/**
 * @file rpg_engine.h
 * @brief Main entry point for the RPG Framework.
 * * Include this file to gain access to all core modules (Camera, Entities, Collision).
 * All framework components are contained within the 'rpg' namespace.
 */

#include "bn_core.h"

// Core Modules
#include "rpg_camera.h"

namespace rpg {
    /**
     * @brief High-level engine utilities.
     */
    inline void init() {
        // Reserved for future global initialization logic
    }
}