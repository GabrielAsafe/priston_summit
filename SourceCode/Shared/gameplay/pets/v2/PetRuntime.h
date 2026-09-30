#pragma once

#include <cstdint>

#include "gameplay/core/v2/GameplayIds.h"

namespace gameplay
{
namespace pets
{
enum class PetRuntimeKind
{
    PcBang,
    Phoenix,
    Help
};

struct PetAreaSync
{
    PetRuntimeKind kind;
    std::uint32_t opcode;
    std::uint32_t w;
    std::uint32_t l;
    std::uint32_t s;
    std::uint32_t e;

    PetAreaSync(PetRuntimeKind kindValue = PetRuntimeKind::PcBang,
                std::uint32_t opcodeValue = 0u,
                std::uint32_t wValue = 0u,
                std::uint32_t lValue = 0u,
                std::uint32_t sValue = 0u,
                std::uint32_t eValue = 0u)
        : kind(kindValue), opcode(opcodeValue), w(wValue), l(lValue), s(sValue), e(eValue) {}
};

struct PetRuntimeSnapshot
{
    bool online;
    std::uint32_t pcBangRoom;
    std::uint32_t phoenixRemaining;
    std::uint32_t helpRemaining;

    PetRuntimeSnapshot(bool onlineValue = false,
                       std::uint32_t pcBangRoomValue = 0u,
                       std::uint32_t phoenixRemainingValue = 0u,
                       std::uint32_t helpRemainingValue = 0u)
        : online(onlineValue),
          pcBangRoom(pcBangRoomValue),
          phoenixRemaining(phoenixRemainingValue),
          helpRemaining(helpRemainingValue) {}

    bool PhoenixActive() const { return phoenixRemaining > 0u; }
    bool HelpActive() const { return helpRemaining > 0u; }
};

enum class PetSyncStatus
{
    Applied,
    InvalidPlayer,
    InvalidChecksum
};

enum class PetPublishStatus
{
    Published,
    Skipped,
    InvalidPlayer
};

class IPetRuntime
{
public:
    virtual ~IPetRuntime() {}
    virtual PetRuntimeSnapshot Snapshot(PlayerId player) const = 0;
    virtual PetSyncStatus ApplyAreaSync(PlayerId player, const PetAreaSync& sync) = 0;
    virtual PetPublishStatus PublishAreaSync(PlayerId player, PetRuntimeKind kind) = 0;
};
}
}
