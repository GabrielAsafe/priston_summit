#include "gameplay/quests/v2/QuestProgressService.h"

#include <cassert>

namespace
{
class FakeProgressService final : public gameplay::quests::IQuestProgressService
{
public:
    bool RecordMonsterKill(gameplay::PlayerId player,
                           gameplay::EntityId monster,
                           bool partyCredit) override
    {
        lastPlayer = player;
        lastMonster = monster;
        lastPartyCredit = partyCredit;
        ++calls;
        return result;
    }

    bool RecordTier5MonsterKill(gameplay::PlayerId killer,
                                gameplay::EntityId monster,
                                const std::vector<gameplay::PlayerId>& damageUsers) override
    {
        lastTier5Killer = killer;
        lastTier5Monster = monster;
        lastDamageUsers = damageUsers;
        ++tier5Calls;
        return result;
    }

    gameplay::PlayerId lastPlayer;
    gameplay::EntityId lastMonster;
    bool lastPartyCredit = false;
    int calls = 0;
    gameplay::PlayerId lastTier5Killer;
    gameplay::EntityId lastTier5Monster;
    std::vector<gameplay::PlayerId> lastDamageUsers;
    int tier5Calls = 0;
    bool result = true;
};
}

int main()
{
    FakeProgressService service;
    assert(service.RecordMonsterKill(gameplay::PlayerId(7), gameplay::EntityId(9), true));
    assert(service.calls == 1);
    assert(service.lastPlayer.value == 7u);
    assert(service.lastMonster.value == 9u);
    assert(service.lastPartyCredit);

    std::vector<gameplay::PlayerId> damageUsers;
    damageUsers.push_back(gameplay::PlayerId(11));
    damageUsers.push_back(gameplay::PlayerId(12));
    assert(service.RecordTier5MonsterKill(
        gameplay::PlayerId(7), gameplay::EntityId(9), damageUsers));
    assert(service.tier5Calls == 1);
    assert(service.lastTier5Killer.value == 7u);
    assert(service.lastTier5Monster.value == 9u);
    assert(service.lastDamageUsers.size() == 2u);
    assert(service.lastDamageUsers[0].value == 11u);
    assert(service.lastDamageUsers[1].value == 12u);
    return 0;
}
