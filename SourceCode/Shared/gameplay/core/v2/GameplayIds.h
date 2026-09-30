#pragma once

#include <cstdint>

namespace gameplay
{
struct PlayerId
{
    std::uint32_t value;
    explicit constexpr PlayerId(std::uint32_t value = 0u) : value(value) {}
    constexpr bool Valid() const { return value != 0u; }
};

inline constexpr bool operator==(PlayerId lhs, PlayerId rhs) { return lhs.value == rhs.value; }
inline constexpr bool operator!=(PlayerId lhs, PlayerId rhs) { return !(lhs == rhs); }
inline constexpr bool operator<(PlayerId lhs, PlayerId rhs) { return lhs.value < rhs.value; }

struct EntityId
{
    std::uint32_t value;
    explicit constexpr EntityId(std::uint32_t value = 0u) : value(value) {}
    constexpr bool Valid() const { return value != 0u; }
};

inline constexpr bool operator==(EntityId lhs, EntityId rhs) { return lhs.value == rhs.value; }
inline constexpr bool operator!=(EntityId lhs, EntityId rhs) { return !(lhs == rhs); }
inline constexpr bool operator<(EntityId lhs, EntityId rhs) { return lhs.value < rhs.value; }

struct MapId
{
    static constexpr std::uint32_t InvalidValue = 0xFFFFFFFFu;
    std::uint32_t value;
    explicit constexpr MapId(std::uint32_t value = InvalidValue) : value(value) {}
    constexpr bool Valid() const { return value != InvalidValue; }
};

inline constexpr bool operator==(MapId lhs, MapId rhs) { return lhs.value == rhs.value; }
inline constexpr bool operator!=(MapId lhs, MapId rhs) { return !(lhs == rhs); }
inline constexpr bool operator<(MapId lhs, MapId rhs) { return lhs.value < rhs.value; }

struct ItemId
{
    std::uint32_t value;
    explicit constexpr ItemId(std::uint32_t value = 0u) : value(value) {}
    constexpr bool Valid() const { return value != 0u; }
};

struct MonsterId
{
    std::uint32_t value;
    explicit constexpr MonsterId(std::uint32_t value = 0u) : value(value) {}
    constexpr bool Valid() const { return value != 0u; }
};

struct NpcId
{
    std::uint32_t value;
    explicit constexpr NpcId(std::uint32_t value = 0u) : value(value) {}
    constexpr bool Valid() const { return value != 0u; }
};

struct QuestId
{
    std::uint32_t value;
    explicit constexpr QuestId(std::uint32_t value = 0u) : value(value) {}
    constexpr bool Valid() const { return value != 0u; }
};

// Runtime serials are recyclable. A handle captures the connection generation
// so long-lived users can reject a stale serial after reconnect/reuse. Generation
// zero is valid: the legacy server assigns dwConnectCount before its first increment.
struct PlayerHandle
{
    PlayerId id;
    std::uint32_t generation;

    constexpr PlayerHandle(PlayerId idValue = PlayerId(), std::uint32_t generationValue = 0u)
        : id(idValue), generation(generationValue) {}

    constexpr bool Valid() const { return id.Valid(); }
};

struct EntityHandle
{
    EntityId id;
    std::uint32_t generation;

    constexpr EntityHandle(EntityId idValue = EntityId(), std::uint32_t generationValue = 0u)
        : id(idValue), generation(generationValue) {}

    constexpr bool Valid() const { return id.Valid(); }
};
}
