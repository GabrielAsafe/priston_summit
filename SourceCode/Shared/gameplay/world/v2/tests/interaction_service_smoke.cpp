#include <cassert>

#include "gameplay/world/v2/IInteractionService.h"

namespace
{
class FakeInteractionService final : public gameplay::world::IInteractionService
{
public:
    gameplay::world::InteractionResult Interact(
        const gameplay::world::InteractionRequest& request) override
    {
        if (!request.player.Valid())
            return gameplay::world::InteractionResult(
                gameplay::world::InteractionStatus::InvalidPlayer, 0u);
        if (!request.target.Valid())
            return gameplay::world::InteractionResult(
                gameplay::world::InteractionStatus::InvalidTarget, 0u);
        return gameplay::world::InteractionResult(
            gameplay::world::InteractionStatus::Handled, 988u);
    }
};
}

int main()
{
    FakeInteractionService service;
    const gameplay::world::InteractionResult handled = service.Interact(
        gameplay::world::InteractionRequest(gameplay::PlayerId(5u), gameplay::EntityId(7u)));
    assert(handled.Handled());
    assert(handled.eventCode == 988u);

    const gameplay::world::InteractionResult invalid = service.Interact(
        gameplay::world::InteractionRequest(gameplay::PlayerId(), gameplay::EntityId(7u)));
    assert(invalid.status == gameplay::world::InteractionStatus::InvalidPlayer);
    return 0;
}
