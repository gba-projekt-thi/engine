#pragma once

#include "bn_log.h"

// =============================================================================
// Standardized logging categories and severity levels.
//
// Wraps BN_LOG with a consistent "[LEVEL][Category] message" prefix so log
// output can be grepped/filtered by subsystem instead of ad-hoc strings.
//
// Usage:
//   BN_LOG_ERROR(LogCategory::Collision, "MAX_STATIC_BODIES exceeded, body dropped");
//   -> "[ERROR][Collision] MAX_STATIC_BODIES exceeded, body dropped"
// =============================================================================

enum class LogCategory {
    Collision,
    Sprite,
    Level,
    Trigger,
    Trap,
    Audio,
    Scene,
    Save,
};

constexpr auto log_category_name(LogCategory category) -> const char* {
    switch(category) {
    case LogCategory::Collision: return "Collision";
    case LogCategory::Sprite:    return "Sprite";
    case LogCategory::Level:     return "Level";
    case LogCategory::Trigger:   return "Trigger";
    case LogCategory::Trap:      return "Trap";
    case LogCategory::Audio:     return "Audio";
    case LogCategory::Scene:     return "Scene";
    case LogCategory::Save:      return "Save";
    default:                     return "Unknown";
    }

    return "Unknown";
}

#define BN_LOG_ERROR(category, ...) \
    BN_LOG("[ERROR][", log_category_name(category), "] ", __VA_ARGS__)

#define BN_LOG_WARN(category, ...) \
    BN_LOG("[WARN][", log_category_name(category), "] ", __VA_ARGS__)

#define BN_LOG_INFO(category, ...) \
    BN_LOG("[INFO][", log_category_name(category), "] ", __VA_ARGS__)
