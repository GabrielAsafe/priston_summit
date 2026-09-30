#include <cassert>

#include "gameplay/covenants/v2/CovenantUIIngress.h"

namespace
{
class FakeCovenantUIIngress final : public gameplay::covenants::ICovenantUIIngress
{
public:
    FakeCovenantUIIngress() : calls(0), lastOwner() {}

    void HandleAction(gameplay::PlayerId owner,
                      const gameplay::covenants::protocol::CovenantUIActionPacket& packet) override
    {
        ++calls;
        lastOwner = owner;
        lastAction = packet.action;
    }

    int calls;
    gameplay::PlayerId lastOwner;
    std::int32_t lastAction;
};
}

int main()
{
    FakeCovenantUIIngress ingress;
    gameplay::covenants::protocol::CovenantUIActionPacket packet{};
    packet.action = gameplay::covenants::protocol::COVENANT_UI_REFRESH;
    ingress.HandleAction(gameplay::PlayerId(77), packet);
    assert(ingress.calls == 1);
    assert(ingress.lastOwner == gameplay::PlayerId(77));
    assert(ingress.lastAction == gameplay::covenants::protocol::COVENANT_UI_REFRESH);
    return 0;
}
