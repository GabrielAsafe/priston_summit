#include <cassert>
#include "gameplay/monsters/v2/SummonedActorCommands.h"

class FakeCommands final : public gameplay::monsters::ISummonedActorCommands
{
public:
    gameplay::monsters::SummonedActorConfigureResult Configure(
        const gameplay::monsters::SummonedActorConfigureRequest& request) override
    {
        gameplay::monsters::SummonedActorConfigureResult result;
        result.actor = request.actor;
        if(!request.actor.Valid() || request.profile.life <= 0)
            return result;
        result.appliedLife = request.profile.life;
        result.code = gameplay::monsters::SummonedActorCommandCode::Ok;
        return result;
    }
};

int main()
{
    gameplay::monsters::SummonedActorProfile profile;
    profile.expiresAtMs = 5000u;
    profile.attackDamageMin = 10;
    profile.attackDamageMax = 20;
    profile.attackRating = 30;
    profile.life = 400;
    profile.overrideDefence = true;
    profile.defence = 50;
    FakeCommands commands;
    const gameplay::monsters::SummonedActorConfigureResult result = commands.Configure(
        gameplay::monsters::SummonedActorConfigureRequest(gameplay::EntityId(77u), profile));
    assert(result.Ok());
    assert(result.actor.value == 77u);
    assert(result.appliedLife == 400);
    return 0;
}
