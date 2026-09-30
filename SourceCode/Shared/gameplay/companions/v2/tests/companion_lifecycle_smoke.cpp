#include "gameplay/companions/v2/CompanionLifecycle.h"

#include <cassert>
#include <iostream>

namespace
{
class FakeLifecycle final : public gameplay::companions::ICompanionLifecycle
{
public:
    void OwnerDisconnected(gameplay::PlayerHandle owner) override
    {
        last = owner;
        ++calls;
    }
    gameplay::PlayerHandle last;
    int calls = 0;
};
}

int main()
{
    FakeLifecycle lifecycle;
    lifecycle.OwnerDisconnected(gameplay::PlayerHandle(gameplay::PlayerId(42u), 7u));
    assert(lifecycle.calls == 1);
    assert(lifecycle.last.id == gameplay::PlayerId(42u));
    assert(lifecycle.last.generation == 7u);
    std::cout << "companion lifecycle smoke: PASS\n";
    return 0;
}
