#pragma once

#include "gameplay/world/v2/ISpawnService.h"
#include "systems/gameplay/player/v2/legacy/LegacyOnlinePlayerRegistry.h"

namespace gameplay
{
namespace world
{
namespace legacy
{
class LegacySpawnAdapter final : public ISpawnService
{
public:
    explicit LegacySpawnAdapter(const player::legacy::LegacyOnlinePlayerRegistry& players);

    SpawnResult Spawn(const SpawnRequest& request) override;

private:
    const player::legacy::LegacyOnlinePlayerRegistry& players_;
};

LegacySpawnAdapter& ServerLegacySpawnAdapter();
}
}
}
