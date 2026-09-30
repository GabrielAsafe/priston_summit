#include "gameplay/companions/v2/CompanionCommands.h"

#include <cassert>
#include <iostream>

namespace
{
class FakeCompanionCommands final : public gameplay::companions::ICompanionCommands
{
public:
    int Select(gameplay::PlayerId owner, const std::string& name) override { return owner.Valid() && !name.empty() ? 1 : 0; }
    int Summon(gameplay::PlayerId owner, const std::string& name) override { return owner.Valid() && !name.empty() ? 2 : 0; }
    int SummonSelected(gameplay::PlayerId owner) override { return owner.Valid() ? 3 : 0; }
    int Dismiss(gameplay::PlayerId owner, const std::string& name) override { return owner.Valid() && !name.empty() ? 4 : 0; }
    int DismissAll(gameplay::PlayerId owner) override { return owner.Valid() ? 5 : 0; }
    int SetMode(gameplay::PlayerId owner, gameplay::companions::CompanionBehaviorMode mode) override
    {
        return owner.Valid() ? 10 + static_cast<int>(mode) : 0;
    }
    int UseCrystal(gameplay::PlayerId owner, std::uint32_t itemCode) override
    {
        return owner.Valid() && itemCode == 1234u ? 20 : 0;
    }
};
}

int main()
{
    FakeCompanionCommands commands;
    const gameplay::PlayerId player(7u);
    assert(commands.Select(player, "alpha") == 1);
    assert(commands.Summon(player, "alpha") == 2);
    assert(commands.SummonSelected(player) == 3);
    assert(commands.Dismiss(player, "alpha") == 4);
    assert(commands.DismissAll(player) == 5);
    assert(commands.SetMode(player, gameplay::companions::CompanionBehaviorMode::Passive) == 12);
    assert(commands.UseCrystal(player, 1234u) == 20);
    assert(commands.Summon(gameplay::PlayerId(), "alpha") == 0);
    std::cout << "companion commands smoke: PASS\n";
    return 0;
}
