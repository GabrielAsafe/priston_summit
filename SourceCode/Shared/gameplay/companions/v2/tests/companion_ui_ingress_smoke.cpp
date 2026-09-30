#include "gameplay/companions/v2/CompanionUIIngress.h"

#include <cassert>
#include <iostream>

namespace
{
class FakeUIIngress final : public gameplay::companions::ICompanionUIIngress
{
public:
    void HandleAction(gameplay::PlayerId owner, const gameplay::companions::protocol::CompanionUIActionPacket& packet) override
    {
        lastOwner = owner;
        lastAction = packet.action;
        ++calls;
    }
    gameplay::PlayerId lastOwner;
    std::int32_t lastAction = -1;
    int calls = 0;
};
}

int main()
{
    FakeUIIngress ingress;
    gameplay::companions::protocol::CompanionUIActionPacket packet{};
    packet.action = gameplay::companions::protocol::COMPANION_UI_REFRESH;
    ingress.HandleAction(gameplay::PlayerId(9u), packet);
    assert(ingress.calls == 1);
    assert(ingress.lastOwner == gameplay::PlayerId(9u));
    assert(ingress.lastAction == gameplay::companions::protocol::COMPANION_UI_REFRESH);
    std::cout << "companion ui ingress smoke: PASS\n";
    return 0;
}
