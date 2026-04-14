#pragma once

#include <cstdint>
#include <cstring>

namespace engine {
namespace save {

/**
 * @brief Static byte buffer for serialization/deserialization.
 *
 * Provides type-safe write/read methods for arbitrary POD types.
 * The maximum size is configurable via a template parameter.
 *
 * Usage (serialization):
 * @code
 *   SaveBuffer<256> buf;
 *   buf.write(player_x);
 *   buf.write(player_y);
 *   buf.write(score);
 * @endcode
 *
 * Usage (deserialization):
 * @code
 *   SaveBuffer<256> buf;
 *   // ... buf.data was read from SRAM ...
 *   buf.read(player_x);
 *   buf.read(player_y);
 *   buf.read(score);
 * @endcode
 */
template<int MaxSize = 512>
class SaveBuffer
{
public:
    /** Raw data */
    uint8_t data[MaxSize];

    /** Current write position / readable data size */
    int size = 0;

    /** Current read position */
    int read_offset = 0;

    /**
     * @brief Writes a POD type to the buffer.
     * @tparam T Trivially-copyable type
     * @param value Value to write
     * @return true on success, false if the buffer is full
     */
    template<typename T>
    bool write(const T& value)
    {
        int len = static_cast<int>(sizeof(T));
        if(size + len > MaxSize)
        {
            return false;
        }
        std::memcpy(data + size, &value, len);
        size += len;
        return true;
    }

    /**
        * @brief Writes a raw byte array to the buffer.
        * @param src Source pointer
        * @param len Number of bytes
        * @return true on success, false if the buffer is full
     */
    bool write_bytes(const void* src, int len)
    {
        if(size + len > MaxSize)
        {
            return false;
        }
        std::memcpy(data + size, src, len);
        size += len;
        return true;
    }

    /**
        * @brief Reads a POD type from the buffer.
        * @tparam T Trivially-copyable type
        * @param value Output for the read value
        * @return true on success, false if not enough data is available
     */
    template<typename T>
    bool read(T& value)
    {
        int len = static_cast<int>(sizeof(T));
        if(read_offset + len > size)
        {
            return false;
        }
        std::memcpy(&value, data + read_offset, len);
        read_offset += len;
        return true;
    }

    /**
        * @brief Reads a raw byte array from the buffer.
        * @param dst Destination pointer
        * @param len Number of bytes
        * @return true on success, false if not enough data is available
     */
    bool read_bytes(void* dst, int len)
    {
        if(read_offset + len > size)
        {
            return false;
        }
        std::memcpy(dst, data + read_offset, len);
        read_offset += len;
        return true;
    }

    /**
        * @brief Resets the buffer.
     */
    void clear()
    {
        size = 0;
        read_offset = 0;
    }

    /**
        * @brief Resets only the read position.
     */
    void reset_read()
    {
        read_offset = 0;
    }

    /**
        * @brief Checks whether the buffer is empty.
     */
    bool empty() const
    {
        return size == 0;
    }

    /**
        * @brief Returns the remaining space in bytes.
     */
    int remaining() const
    {
        return MaxSize - size;
    }

    /**
        * @brief Returns the maximum capacity.
     */
    constexpr int capacity() const
    {
        return MaxSize;
    }
};

} // namespace save
} // namespace engine