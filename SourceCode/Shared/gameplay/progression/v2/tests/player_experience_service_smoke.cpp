#include "gameplay/progression/v2/PlayerExperienceService.h"

#include <cassert>

namespace
{
class FakeExperiencePort final : public gameplay::progression::IPlayerExperienceMutationPort
{
public:
    int calls = 0;
    gameplay::progression::PlayerExperienceGrantRequest last;

    gameplay::progression::PlayerExperienceGrantResult Grant(
        const gameplay::progression::PlayerExperienceGrantRequest& request) override
    {
        ++calls;
        last = request;
        gameplay::progression::PlayerExperienceGrantResult result;
        result.code = gameplay::progression::PlayerExperienceGrantCode::Granted;
        result.player = request.player;
        result.amount = request.amount;
        return result;
    }
};
}

int main()
{
    using namespace gameplay;
    using namespace gameplay::progression;

    FakeExperiencePort port;
    PlayerExperienceGrantRequest request(
        PlayerId(7u), 1250, EntityId(99u), true, PlayerExperienceSource::MonsterParty);
    PlayerExperienceGrantResult result = PlayerExperienceService::Grant(port, request);
    assert(result.Granted());
    assert(port.calls == 1);
    assert(port.last.player == PlayerId(7u));
    assert(port.last.amount == 1250);
    assert(port.last.sourceEntity == EntityId(99u));
    assert(port.last.party);

    result = PlayerExperienceService::Grant(
        port, PlayerExperienceGrantRequest(PlayerId(), 100, EntityId(), false));
    assert(result.code == PlayerExperienceGrantCode::InvalidRequest);
    assert(port.calls == 1);

    result = PlayerExperienceService::Grant(
        port, PlayerExperienceGrantRequest(PlayerId(7u), 0, EntityId(), false));
    assert(result.code == PlayerExperienceGrantCode::InvalidRequest);
    assert(port.calls == 1);
    return 0;
}
