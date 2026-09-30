#pragma once

#include "gameplay/pets/v2/PetRuntime.h"
#include "systems/gameplay/player/v2/legacy/LegacyOnlinePlayerRegistry.h"

namespace gameplay
{
namespace pets
{
namespace legacy
{
class LegacyPetRuntimeAdapter final : public IPetRuntime
{
public:
    explicit LegacyPetRuntimeAdapter(const player::legacy::LegacyOnlinePlayerRegistry& players);
    PetRuntimeSnapshot Snapshot(PlayerId player) const override;
    PetSyncStatus ApplyAreaSync(PlayerId player, const PetAreaSync& sync) override;
    PetPublishStatus PublishAreaSync(PlayerId player, PetRuntimeKind kind) override;

private:
    const player::legacy::LegacyOnlinePlayerRegistry& players_;
};

LegacyPetRuntimeAdapter& ServerLegacyPetRuntimeAdapter();
}
}
}
