#include "gameplay/companions/v2/CompanionQuery.h"

#include <cassert>
#include <iostream>
#include <map>

namespace
{
class FakeCompanionQuery final : public gameplay::companions::ICompanionQuery
{
public:
    std::uint32_t ActiveCount(gameplay::PlayerId owner) const override
    {
        const auto found = counts.find(owner.value);
        return found == counts.end() ? 0u : found->second;
    }
    bool TryGetState(gameplay::PlayerId owner, gameplay::companions::CompanionStateSnapshot& out) const override
    {
        out = gameplay::companions::CompanionStateSnapshot();
        out.activeCount = ActiveCount(owner);
        return owner.Valid();
    }
    std::uint32_t ConfiguredCrystalItemCode() const override { return 1234u; }
    std::map<std::uint32_t, std::uint32_t> counts;
};
}

int main()
{
    FakeCompanionQuery query;
    query.counts[7u] = 3u;
    assert(query.ActiveCount(gameplay::PlayerId(7u)) == 3u);
    assert(query.ActiveCount(gameplay::PlayerId(8u)) == 0u);
    std::cout << "companion query smoke: PASS\n";
    return 0;
}
