#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

namespace gameplay
{
namespace events
{
struct MonsterKilledEvent;
}

namespace hunting
{
struct HuntingRuntimeProgress
{
    std::uint64_t totalKills;
    std::uint64_t soloKills;
    std::uint64_t partyKills;
    int lastArea;
    std::string lastMonsterName;
    std::unordered_map<std::uint32_t, std::uint64_t> killsByMonsterCode;

    HuntingRuntimeProgress()
        : totalKills(0), soloKills(0), partyKills(0), lastArea(-1)
    {
    }
};

class HuntingService
{
public:
    void OnMonsterKilled(const events::MonsterKilledEvent& event);

    bool GetProgress(const std::string& playerName, HuntingRuntimeProgress& out) const;
    std::uint64_t GetMonsterKills(const std::string& playerName,
                                  std::uint32_t monsterCode) const;
    void ClearPlayer(const std::string& playerName);
    void Clear();

private:
    std::unordered_map<std::string, HuntingRuntimeProgress> progressByPlayer_;
};

HuntingService& ServerHuntingService();
}
}
