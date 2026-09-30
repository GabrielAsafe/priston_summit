#pragma once

#include "gameplay/monsters/v2/MonsterCommands.h"
#include "gameplay/monsters/v2/MonsterQuery.h"

namespace gameplay
{
namespace monsters
{
namespace legacy
{
class LegacyMonsterAdapter final : public IMonsterCommands, public IMonsterQuery
{
public:
    bool TryGetMonster(EntityId entity, MonsterSnapshot& out) const override;
    FollowerConversionResult ConvertToFollower(const FollowerConversionRequest& request) override;
};

LegacyMonsterAdapter& ServerLegacyMonsterAdapter();
}
}
}
