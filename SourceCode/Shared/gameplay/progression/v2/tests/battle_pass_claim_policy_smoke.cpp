#include "Shared/gameplay/progression/v2/BattlePassClaimPolicy.h"
#include <cassert>
#include <iostream>

int main()
{
    using namespace gameplay::progression;
    BattlePassClaimContext c{};
    c.rewardTag = 0;
    c.passType = 1;
    c.playerLevel = 10;
    c.requiredLevel = 10;
    assert(BattlePassClaimPolicy::Evaluate(c) == BattlePassClaimDecision::Allowed);

    c.rewardTag = 1;
    c.passType = 1;
    assert(BattlePassClaimPolicy::Evaluate(c) == BattlePassClaimDecision::PassNotEntitled);
    c.passType = 2;
    assert(BattlePassClaimPolicy::Evaluate(c) == BattlePassClaimDecision::Allowed);

    c.playerLevel = 9;
    assert(BattlePassClaimPolicy::Evaluate(c) == BattlePassClaimDecision::LevelTooLow);
    c.playerLevel = 10;
    c.alreadyClaimed = true;
    assert(BattlePassClaimPolicy::Evaluate(c) == BattlePassClaimDecision::AlreadyClaimed);

    std::cout << "battle pass claim policy smoke: PASS\n";
    return 0;
}
