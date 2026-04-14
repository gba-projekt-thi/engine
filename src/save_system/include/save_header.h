#pragma once

#include <cstdint>

namespace engine {
namespace save {

/**
 * @brief Header for each save slot in SRAM.
 *
 * Stored at the beginning of each slot and contains metadata
 * for validation and versioning.
 */
struct SaveHeader
{
    /** Magic number used to detect valid save data ("GBAS" = 0x53414247 little-endian) */
    uint32_t magic;

    /** Schema version - increment on breaking changes */
    uint16_t version;

    /** Slot ID (0-based) */
    uint16_t slot_id;

    /** CRC32 checksum of the payload (excluding the header) */
    uint32_t checksum;

    /** Payload size in bytes */
    uint32_t payload_size;
};

/** Magic number constant */
constexpr uint32_t SAVE_MAGIC = 0x53414247; // "GBAS" in little-endian

/** Current schema version */
constexpr uint16_t SAVE_VERSION = 1;

} // namespace save
} // namespace engine