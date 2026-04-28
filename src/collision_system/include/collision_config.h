#pragma once

#include <cstdint>

constexpr uint16_t MASK_PLAYER  = 0x0001;
constexpr uint16_t MASK_ENEMY   = 0x0002;
constexpr uint16_t MASK_DOOR    = 0x0004;
constexpr uint16_t MASK_TILEMAP = 0x8000;

constexpr uint8_t TYPE_STATIC_BODY  = 0;
constexpr uint8_t TYPE_PHYSICS_BODY = 1;
constexpr uint8_t TYPE_PLAYER       = 2;
constexpr uint8_t TYPE_ENEMY        = 3;
