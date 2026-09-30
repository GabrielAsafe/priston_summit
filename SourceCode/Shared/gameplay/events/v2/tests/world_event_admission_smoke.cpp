#include <cassert>
#include "gameplay/events/v2/WorldEventAdmission.h"

namespace
{
class FakeAdmission final : public gameplay::events::IWorldEventAdmissionService
{
public:
    gameplay::events::WorldEventAdmissionStatus Check(
        const gameplay::events::WorldEventAdmissionRequest& request) const override
    {
        if(!request.player.Valid())
            return gameplay::events::WorldEventAdmissionStatus::PlayerUnavailable;
        return request.singleCharacterPerRemoteAddress
            ? gameplay::events::WorldEventAdmissionStatus::RemoteAddressAlreadyPresent
            : gameplay::events::WorldEventAdmissionStatus::Allowed;
    }
};
}

int main()
{
    FakeAdmission admission;
    assert(admission.Check(gameplay::events::WorldEventAdmissionRequest(
        gameplay::PlayerId(), gameplay::MapId(23u), true)) ==
        gameplay::events::WorldEventAdmissionStatus::PlayerUnavailable);
    assert(admission.Check(gameplay::events::WorldEventAdmissionRequest(
        gameplay::PlayerId(7u), gameplay::MapId(23u), false)) ==
        gameplay::events::WorldEventAdmissionStatus::Allowed);
    return 0;
}
