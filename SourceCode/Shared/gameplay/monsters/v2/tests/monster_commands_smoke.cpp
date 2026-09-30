#include "gameplay/monsters/v2/MonsterCommands.h"

#include <cassert>

namespace
{
class FakeMonsterCommands final : public gameplay::monsters::IMonsterCommands
{
public:
    gameplay::monsters::FollowerConversionResult ConvertToFollower(
        const gameplay::monsters::FollowerConversionRequest& request) override
    {
        gameplay::monsters::FollowerConversionResult result;
        if(!request.target.Valid() || !request.owner.Valid() || request.durationTicks <= 0 || request.lifePercent <= 0)
            return result;
        result.code = gameplay::monsters::MonsterCommandCode::Ok;
        result.previousBrood = 3;
        result.previousLife = 200;
        result.convertedLife = result.previousLife * request.lifePercent / 100;
        result.durationTicks = request.durationTicks;
        result.converted = true;
        return result;
    }
};
}

int main()
{
    FakeMonsterCommands commands;
    const gameplay::monsters::FollowerConversionResult converted = commands.ConvertToFollower(
        gameplay::monsters::FollowerConversionRequest(gameplay::EntityId(10u), gameplay::PlayerId(20u), 170, 50));
    assert(converted.Ok());
    assert(converted.previousLife == 200);
    assert(converted.convertedLife == 100);
    assert(converted.durationTicks == 170);

    const gameplay::monsters::FollowerConversionResult rejected = commands.ConvertToFollower(
        gameplay::monsters::FollowerConversionRequest());
    assert(!rejected.Ok());
    return 0;
}
