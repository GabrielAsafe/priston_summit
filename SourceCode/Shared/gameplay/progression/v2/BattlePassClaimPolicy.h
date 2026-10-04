#pragma once

namespace gameplay::progression
{
enum class BattlePassClaimDecision
{
    Allowed,
    PassNotEntitled,
    LevelTooLow,
    AlreadyClaimed
};

struct BattlePassClaimContext
{
    int rewardTag = 0;
    int passType = 0;
    int playerLevel = 0;
    int requiredLevel = 0;
    bool alreadyClaimed = false;
};

class BattlePassClaimPolicy
{
public:
    static BattlePassClaimDecision Evaluate(const BattlePassClaimContext& context) noexcept
    {
        const bool premiumLane = (context.rewardTag % 2) != 0;
        const bool entitled = premiumLane ? (context.passType == 2) : (context.passType > 0);
        if (!entitled)
            return BattlePassClaimDecision::PassNotEntitled;
        if (context.playerLevel < context.requiredLevel)
            return BattlePassClaimDecision::LevelTooLow;
        if (context.alreadyClaimed)
            return BattlePassClaimDecision::AlreadyClaimed;
        return BattlePassClaimDecision::Allowed;
    }
};
}
