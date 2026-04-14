#pragma once

#include "save_header.h"
#include <cstdint>

namespace engine {
namespace save {

/**
 * @brief Calculates a CRC32 checksum.
 *
 * Uses the standard polynomial 0xEDB88320 (IEEE 802.3).
 * Table-free implementation to save RAM on the GBA.
 *
 * @param data Pointer to the data
 * @param length Length in bytes
 * @return CRC32 checksum
 */
uint32_t crc32(const uint8_t* data, int length);

/**
 * @brief Validates the header of a save slot.
 *
 * Checks magic number and version.
 *
 * @param header The header to validate
 * @return true if the header is valid
 */
bool validate_header(const SaveHeader& header);

/**
 * @brief Validates the integrity of a save payload.
 *
 * Compares the checksum stored in the header with a
 * newly computed checksum over the payload.
 *
 * @param header Header containing the stored checksum
 * @param payload Pointer to the payload
 * @param payload_size Payload size in bytes
 * @return true if the checksum matches
 */
bool validate_checksum(const SaveHeader& header, const uint8_t* payload, int payload_size);

} // namespace save
} // namespace engine