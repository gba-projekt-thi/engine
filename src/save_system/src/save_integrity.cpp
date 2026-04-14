#include "save_integrity.h"

namespace engine {
namespace save {

uint32_t crc32(const uint8_t* data, int length)
{
    // Initialize with all bits set, then process each byte bit-by-bit.
    uint32_t crc = 0xFFFFFFFF;

    for(int i = 0; i < length; ++i)
    {
        crc ^= data[i];

        // Table-free CRC32 update using polynomial 0xEDB88320.
        for(int j = 0; j < 8; ++j)
        {
            uint32_t mask = -(crc & 1);
            crc = (crc >> 1) ^ (0xEDB88320 & mask);
        }
    }

    // Final XOR to produce the standard CRC32 output.
    return ~crc;
}

bool validate_header(const SaveHeader& header)
{
    // Reject empty/corrupt slots by checking expected magic first.
    if(header.magic != SAVE_MAGIC)
    {
        return false;
    }

    // Accept only known schema versions in the supported range.
    if(header.version == 0 || header.version > SAVE_VERSION)
    {
        return false;
    }

    return true;
}

bool validate_checksum(const SaveHeader& header, const uint8_t* payload, int payload_size)
{
    // Empty payload is considered invalid for integrity checks.
    if(payload_size <= 0)
    {
        return false;
    }

    // Recompute checksum and compare it against the stored value.
    uint32_t computed = crc32(payload, payload_size);
    return header.checksum == computed;
}

} // namespace save
} // namespace engine