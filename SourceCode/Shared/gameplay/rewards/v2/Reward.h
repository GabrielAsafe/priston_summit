#pragma once

#include <string>
#include <vector>

namespace gameplay
{
namespace rewards
{
enum class RewardType
{
    Item,
    Gold,
    Experience,
    Reputation,
    StoryFlag,
    CovenantReputation,
    Custom
};

struct RewardDefinition
{
    RewardType type;
    std::string key;
    int id;
    int amount;

    RewardDefinition()
        : type(RewardType::Custom), id(0), amount(0)
    {
    }
};

enum class RewardResult
{
    Granted,
    Unsupported,
    Rejected,
    Failed
};

class IRewardGateway
{
public:
    virtual ~IRewardGateway() {}
    virtual bool Supports(RewardType) const { return true; }
    virtual bool Validate(const RewardDefinition& reward) const { return Supports(reward.type); }
    virtual bool IsRetrySafe(RewardType) const { return false; }
    virtual bool IsRetrySafe(const RewardDefinition& reward) const { return IsRetrySafe(reward.type); }
    virtual RewardResult Grant(const std::string& playerKey,
                               const RewardDefinition& reward,
                               const std::string& sourceKey) = 0;
};

class RewardService
{
public:
    explicit RewardService(IRewardGateway& gateway) : gateway_(gateway) {}

    bool Supports(const RewardDefinition& reward) const
    {
        return gateway_.Supports(reward.type);
    }

    bool Validate(const RewardDefinition& reward) const
    {
        return gateway_.Validate(reward);
    }

    bool IsRetrySafe(const RewardDefinition& reward) const
    {
        return gateway_.IsRetrySafe(reward);
    }

    RewardResult Grant(const std::string& playerKey,
                       const RewardDefinition& reward,
                       const std::string& sourceKey)
    {
        if (playerKey.empty() || sourceKey.empty())
            return RewardResult::Rejected;
        if (!Supports(reward))
            return RewardResult::Unsupported;
        if (!Validate(reward))
            return RewardResult::Rejected;
        return gateway_.Grant(playerKey, reward, sourceKey);
    }

    RewardResult GrantAll(const std::string& playerKey,
                          const std::vector<RewardDefinition>& rewards,
                          const std::string& sourceKey)
    {
        for (std::vector<RewardDefinition>::const_iterator it = rewards.begin();
             it != rewards.end(); ++it)
        {
            if (!Supports(*it))
                return RewardResult::Unsupported;
            if (!Validate(*it))
                return RewardResult::Rejected;
        }
        for (std::vector<RewardDefinition>::const_iterator it = rewards.begin();
             it != rewards.end(); ++it)
        {
            const RewardResult result = Grant(playerKey, *it, sourceKey);
            if (result != RewardResult::Granted)
                return result;
        }
        return RewardResult::Granted;
    }

private:
    IRewardGateway& gateway_;
};
}
}
