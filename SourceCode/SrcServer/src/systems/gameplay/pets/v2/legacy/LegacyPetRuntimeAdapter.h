#pragma once

#include "gameplay/pets/v2/PetRuntime.h"
#include "systems/gameplay/player/v2/legacy/LegacyOnlinePlayerRegistry.h"
#include "network/v2/PacketSender.h"

namespace gameplay
{
namespace pets
{
namespace legacy
{
class LegacyPetRuntimeAdapter final : public IPetRuntime
{
public:
    LegacyPetRuntimeAdapter(const player::legacy::LegacyOnlinePlayerRegistry& players,
                            network::IPacketSender& packets);
    PetRuntimeSnapshot Snapshot(PlayerId player) const override;
    PetSyncStatus ApplyAreaSync(PlayerId player, const PetAreaSync& sync) override;
    PetPublishStatus PublishAreaSync(PlayerId player, PetRuntimeKind kind) override;

private:
    const player::legacy::LegacyOnlinePlayerRegistry& players_;
    network::IPacketSender& packets_;
};

LegacyPetRuntimeAdapter& ServerLegacyPetRuntimeAdapter();
}
}
}
