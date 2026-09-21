#include "systems/gameplay/hunting/v2/HuntingService.h"

#include "systems/gameplay/events/v2/MonsterKilledEvent.h"

namespace gameplay
{
namespace hunting
{
void HuntingService::OnMonsterKilled(const events::MonsterKilledEvent& event)
{
    if (event.playerName.empty() || event.monsterName.empty())
        return;

    HuntingRuntimeProgress& progress = progressByPlayer_[event.playerName];
    ++progress.totalKills;
    if (event.partyCredit)
        ++progress.partyKills;
    else
        ++progress.soloKills;

    progress.lastArea = event.area;
    progress.lastMonsterName = event.monsterName;
    ++progress.killsByMonsterCode[event.monsterCode];
}

bool HuntingService::GetProgress(const std::string& playerName,
                                 HuntingRuntimeProgress& out) const
{
    const std::unordered_map<std::string, HuntingRuntimeProgress>::const_iterator it =
        progressByPlayer_.find(playerName);
    if (it == progressByPlayer_.end())
        return false;
    out = it->second;
    return true;
}

std::uint64_t HuntingService::GetMonsterKills(const std::string& playerName,
                                              std::uint32_t monsterCode) const
{
    const std::unordered_map<std::string, HuntingRuntimeProgress>::const_iterator playerIt =
        progressByPlayer_.find(playerName);
    if (playerIt == progressByPlayer_.end())
        return 0;

    const std::unordered_map<std::uint32_t, std::uint64_t>::const_iterator monsterIt =
        playerIt->second.killsByMonsterCode.find(monsterCode);
    return monsterIt == playerIt->second.killsByMonsterCode.end() ? 0 : monsterIt->second;
}

void HuntingService::ClearPlayer(const std::string& playerName)
{
    progressByPlayer_.erase(playerName);
}

void HuntingService::Clear()
{
    progressByPlayer_.clear();
}

HuntingService& ServerHuntingService()
{
    static HuntingService service;
    return service;
}
}
}
