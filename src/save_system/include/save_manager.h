#pragma once

#include "save_header.h"
#include "save_buffer.h"
#include "save_integrity.h"
#include "save_slot.h"
#include "bn_sram.h"

namespace engine {
namespace save {

/**
 * @brief Result codes for save/load operations.
 */
enum class SaveResult
{
    OK,
    SLOT_INVALID,
    SLOT_EMPTY,
    CHECKSUM_FAIL,
    VERSION_MISMATCH,
    SRAM_ERROR,
    SERIALIZE_ERROR
};

/**
 * @brief Generic save manager for GBA games.
 *
 * Reads/writes all slots at once via bn::sram::read/write,
 * because Butano does not provide offset-based SRAM access.
 *
 * @tparam GameState   Game-specific state type (trivially-copyable)
 * @tparam MaxSlots    Number of save slots (default: 3)
 * @tparam BufferSize  Maximum payload size in bytes (default: 512)
 *
 * ## Usage:
 * @code
 *   engine::save::SaveManager<MyState> save_mgr;
 *   save_mgr.save(0, state);
 *
 *   MyState loaded;
 *   if(save_mgr.load(0, loaded) == engine::save::SaveResult::OK) { ... }
 * @endcode
 */
template<typename GameState, int MaxSlots = 3, int BufferSize = 512>
class SaveManager
{
public:
    using Data = SramData<MaxSlots, BufferSize>;

    SaveManager()
    {
        static_assert(sizeof(Data) <= SRAM_TOTAL_SIZE,
                      "Save slots exceed available SRAM (32 KB)");
        static_assert(sizeof(GameState) <= BufferSize,
                      "GameState is larger than BufferSize");
    }

    /**
        * @brief Saves the game state to a slot.
     */
    SaveResult save(int slot_id, const GameState& state)
    {
        if(slot_id < 0 || slot_id >= MaxSlots)
        {
            return SaveResult::SLOT_INVALID;
        }

        // Read entire SRAM contents
        Data sram_data;
        bn::sram::read(sram_data);

        // Serialize payload
        SaveBuffer<BufferSize> buf;
        if(!buf.write(state))
        {
            return SaveResult::SERIALIZE_ERROR;
        }

        // Build header
        SaveSlot<BufferSize>& slot = sram_data.slots[slot_id];
        slot.header.magic        = SAVE_MAGIC;
        slot.header.version      = SAVE_VERSION;
        slot.header.slot_id      = static_cast<uint16_t>(slot_id);
        slot.header.checksum     = crc32(buf.data, buf.size);
        slot.header.payload_size = static_cast<uint32_t>(buf.size);

        // Copy payload into slot
        std::memcpy(slot.payload, buf.data, buf.size);

        // Write entire SRAM contents back
        bn::sram::write(sram_data);

        return SaveResult::OK;
    }

    /**
     * @brief Loads the game state from a slot.
     */
    SaveResult load(int slot_id, GameState& state)
    {
        if(slot_id < 0 || slot_id >= MaxSlots)
        {
            return SaveResult::SLOT_INVALID;
        }

        // Read entire SRAM
        Data sram_data;
        bn::sram::read(sram_data);

        const SaveSlot<BufferSize>& slot = sram_data.slots[slot_id];

        // Validate header
        if(!validate_header(slot.header))
        {
            return SaveResult::SLOT_EMPTY;
        }

        // Version check
        if(slot.header.version > SAVE_VERSION)
        {
            return SaveResult::VERSION_MISMATCH;
        }

        // Validate payload size
        int payload_size = static_cast<int>(slot.header.payload_size);
        if(payload_size <= 0 || payload_size > BufferSize)
        {
            return SaveResult::SLOT_EMPTY;
        }

        // Validate checksum
        if(!validate_checksum(slot.header, slot.payload, payload_size))
        {
            return SaveResult::CHECKSUM_FAIL;
        }

        // Deserialize
        SaveBuffer<BufferSize> buf;
        std::memcpy(buf.data, slot.payload, payload_size);
        buf.size = payload_size;

        if(!buf.read(state))
        {
            return SaveResult::SERIALIZE_ERROR;
        }

        return SaveResult::OK;
    }

    /**
        * @brief Erases a save slot.
     */
    SaveResult erase(int slot_id)
    {
        if(slot_id < 0 || slot_id >= MaxSlots)
        {
            return SaveResult::SLOT_INVALID;
        }

        Data sram_data;
        bn::sram::read(sram_data);

        // Zero the slot
        std::memset(&sram_data.slots[slot_id], 0, sizeof(SaveSlot<BufferSize>));

        bn::sram::write(sram_data);

        return SaveResult::OK;
    }

    /**
        * @brief Checks whether a slot contains valid data.
     */
    bool is_slot_used(int slot_id) const
    {
        if(slot_id < 0 || slot_id >= MaxSlots)
        {
            return false;
        }

        Data sram_data;
        bn::sram::read(sram_data);

        return validate_header(sram_data.slots[slot_id].header);
    }

    /**
        * @brief Returns the number of slots.
     */
    constexpr int slot_count() const
    {
        return MaxSlots;
    }

    /**
        * @brief Returns the total size of SRAM data in bytes.
     */
    constexpr int total_sram_size() const
    {
        return static_cast<int>(sizeof(Data));
    }
};

} // namespace save
} // namespace engine