#pragma once

#include "save_header.h"
#include <cstdint>
#include <cstring>

namespace engine {
namespace save {

/**
 * @brief A single save slot: header + raw payload data.
 *
 * @tparam PayloadSize Maximum payload size in bytes
 */
template<int PayloadSize>
struct SaveSlot
{
    SaveHeader header;
    uint8_t    payload[PayloadSize];
};

/**
 * @brief Container for all save slots - written to SRAM as a whole.
 *
 * Butano's bn::sram::read/write can only read/write a single object
 * without offset support. Therefore, all slots are grouped into
 * one struct.
 *
 * @tparam MaxSlots    Number of slots
 * @tparam PayloadSize Maximum payload size per slot
 */
template<int MaxSlots, int PayloadSize>
struct SramData
{
    SaveSlot<PayloadSize> slots[MaxSlots];
};

/** Maximum SRAM size on GBA: 32 KiB */
constexpr int SRAM_TOTAL_SIZE = 32768;

} // namespace save
} // namespace engine