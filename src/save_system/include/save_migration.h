#pragma once

#include <cstdint>
#include <cstring>
#include <type_traits>

namespace engine {
namespace save {

/**
 * Schema versions: engine::save::SAVE_VERSION (save_header.h) is bumped
 * whenever the saved state's layout changes, i.e. a field is appended. The
 * state's ValidBytesFn must then be extended so the previous version still
 * maps to the bytes that existed back then.
 *
 * @brief Tells how many leading bytes of a payload written by schema
 *        `version` are meaningful for the CURRENT state layout.
 *
 * Bytes beyond that prefix are filled with the state's default values.
 * Returning 0 marks a version as not migratable (the slot is treated as
 * incompatible). The function must return sizeof(State) for the current
 * version.
 *
 * Why a version-keyed prefix and not just the stored payload size: a new field
 * can fit into the struct's tail padding, so sizeof(State) does not change and
 * an old payload has the same size as a new one - and its padding byte is
 * indeterminate, not a default value.
 */
using ValidBytesFn = int (*)(uint16_t version);

enum class RestoreResult
{
    /** Payload matches the current layout exactly. */
    Exact,
    /** Older payload: its prefix was copied, newer fields hold defaults. */
    Migrated,
    /** Nothing usable (unknown/too old version, or size mismatch). */
    Incompatible
};

/**
 * @brief Restores a state from a raw payload, migrating older layouts.
 *
 * Pure and hardware-independent (unit-tested on the host). New fields must be
 * APPENDED to the state struct, so every older layout is a prefix of the
 * current one.
 *
 * Without `valid_bytes` the legacy rule applies: the payload must be exactly
 * sizeof(State), anything else is Incompatible.
 *
 * `out` is only written when the result is not Incompatible.
 */
template<typename State>
auto restore_state(const uint8_t* payload,
                   int payload_size,
                   uint16_t version,
                   ValidBytesFn valid_bytes,
                   State& out) -> RestoreResult
{
    static_assert(std::is_trivially_copyable_v<State>,
                  "State must be trivially copyable");

    constexpr int state_size = static_cast<int>(sizeof(State));

    if(valid_bytes == nullptr)
    {
        if(payload_size != state_size)
        {
            return RestoreResult::Incompatible;
        }
        std::memcpy(&out, payload, state_size);
        return RestoreResult::Exact;
    }

    int usable = payload_size;
    const int valid = valid_bytes(version);
    if(valid < usable)
    {
        usable = valid;
    }
    if(usable > state_size)
    {
        usable = state_size;
    }
    if(usable <= 0)
    {
        return RestoreResult::Incompatible;
    }

    State merged{};
    std::memcpy(&merged, payload, usable);
    out = merged;

    return (usable == state_size && payload_size == state_size)
               ? RestoreResult::Exact
               : RestoreResult::Migrated;
}

} // namespace save
} // namespace engine
