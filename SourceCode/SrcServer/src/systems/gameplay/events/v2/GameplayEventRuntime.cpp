#include "systems/gameplay/events/v2/GameplayEventRuntime.h"

#include <cstdlib>
#include <cstring>
#include <memory>
#include <vector>
#include "systems/persistence/database/core/Database.h"
#include <sstream>

#include "systems/gameplay/hunting/v2/HuntingService.h"
#include "systems/gameplay/progression/v2/ProgressionRuntime.h"
#include "systems/gameplay/professions/v2/ProfessionRuntime.h"
#include "gameplay/covenants/v2/CovenantCatalog.h"

namespace gameplay
{
namespace events
{
namespace
{
// 377: optional boss materials, disabled by default. Physical item drops are
// never made here. Every granted SQLite resource has a per-kill receipt.
void TryAwardHunter378(const MonsterKilledEvent& event)
{
    if(!event.monsterBoss || event.playerName.empty() ||
       event.playerName.size()>255 || !event.monster.Valid())return;
    const int level=professions::ServerProfessionService().GetLevel(
        event.playerName, professions::ProfessionType::Hunter);
    std::shared_ptr<SQLConnection> db=SQLConnection::GetConnection(DATABASEID_ServerDB);
    if(!db || !db->Open())return;
    const int monsterCode=static_cast<int>(event.monsterCode);
    struct Rule {std::string key;int chance;int level;};
    std::vector<Rule> rules;
    bool ok=db->Prepare("SELECT ResourceKey,ChancePer10000,MinHunterLevel "
                        "FROM HunterDropRule378 WHERE MonsterCode=? AND Enabled=1") &&
        db->BindInputParameter(const_cast<int*>(&monsterCode),1,PARAMTYPE_Integer) && db->Execute(false);
    while(ok && db->NextRow()) {
        char resource[128]={0};Rule rule;
        ok=db->GetData(resource,sizeof(resource),1,PARAMTYPE_String) &&
            db->GetData(&rule.chance,sizeof(rule.chance),2,PARAMTYPE_Integer) &&
            db->GetData(&rule.level,sizeof(rule.level),3,PARAMTYPE_Integer);
        if(!ok)break;
        rule.key=resource;
        if(rule.key.empty() || rule.key.size()>127 || rules.size()>12) {ok=false;break;}
        rules.push_back(rule);
    }
    db->Close();if(!ok)return;
    for(std::size_t i=0;i<rules.size();++i) {
        const Rule& r=rules[i];
        if(level<r.level || r.chance<=0 || r.chance>10000 ||
           std::rand()%10000>=r.chance)continue;
        std::shared_ptr<SQLConnection> grant=SQLConnection::GetConnection(DATABASEID_ServerDB);
        if(!grant || !grant->Open() || !grant->BeginTransaction())continue;
        char player[256]={0},resource[128]={0};
        std::strncpy(player,event.playerName.c_str(),sizeof(player)-1);
        std::strncpy(resource,r.key.c_str(),sizeof(resource)-1);
        int serial=static_cast<int>(event.monster.value);
        bool wrote=grant->Prepare("INSERT OR IGNORE INTO HunterKillReceipt378 "
            "(PlayerKey,MonsterCode,MonsterSerial,ResourceKey) VALUES(?,?,?,?)") &&
            grant->BindInputParameter(player,1,PARAMTYPE_String) &&
            grant->BindInputParameter(const_cast<int*>(&monsterCode),2,PARAMTYPE_Integer) &&
            grant->BindInputParameter(&serial,3,PARAMTYPE_Integer) &&
            grant->BindInputParameter(resource,4,PARAMTYPE_String) && grant->Execute(false);
        int changed=0;
        wrote=wrote && grant->Prepare("SELECT changes()") && grant->Execute(false) &&
            grant->NextRow() && grant->GetData(&changed,sizeof(changed),1,PARAMTYPE_Integer);
        if(wrote && changed==1) {
            int one=1;
            wrote=grant->Prepare("INSERT INTO GatheringPlayerInventory "
                "(PlayerKey,ResourceKey,Quantity,UpdatedAt) VALUES(?,?,?,datetime('now')) "
                "ON CONFLICT(PlayerKey,ResourceKey) DO UPDATE SET Quantity=Quantity+1, "
                "UpdatedAt=excluded.UpdatedAt WHERE Quantity<2147483647") &&
                grant->BindInputParameter(player,1,PARAMTYPE_String) &&
                grant->BindInputParameter(resource,2,PARAMTYPE_String) &&
                grant->BindInputParameter(&one,3,PARAMTYPE_Integer) && grant->Execute(false);
        }
        if(wrote && grant->CommitTransaction())grant->Close();
        else {grant->RollbackTransaction();grant->Close();}
    }
}

void EnsureCoreConsumersRegistered()
{
    static const GameplayEventHub::SubscriptionId huntingSubscription =
        ServerGameplayEventHub().Subscribe<MonsterKilledEvent>(
            [](const MonsterKilledEvent& event)
            {
                hunting::ServerHuntingService().OnMonsterKilled(event);
                TryAwardHunter378(event);
                if (!event.playerName.empty())
                {
                    // Hunter is a V2-only profession identity. One credited monster kill
                    // advances one persistent Hunter XP unit; no legacy JOB_CODE mapping
                    // is required or permitted for this activation path.
                    (void)professions::ServerProfessionService().AddExperience(
                        event.playerName, professions::ProfessionType::Hunter, 1u);
                }
            });

    static const GameplayEventHub::SubscriptionId covenantSubscription =
        ServerGameplayEventHub().Subscribe<MonsterKilledEvent>(
            [](const MonsterKilledEvent& event)
            {
                if (event.playerName.empty())
                    return;

                (void)progression::RecordServerCovenantActivity(
                    event.playerName, "monster.kill", 1);
                if (event.monsterBoss)
                {
                    (void)progression::RecordServerCovenantActivity(
                        event.playerName, "hunt.kill", 1);

                    covenants::CovenantModifierAggregate trophyChance;
                    if (progression::GetServerCovenantModifierAggregate(
                            event.playerName, "hunt.trophy_chance", "hunt_target",
                            trophyChance, true, event.monster.value) &&
                        trophyChance.matched)
                    {
                        double chance = trophyChance.flat + trophyChance.additivePercent;
                        if (trophyChance.multiplicativeFactor != 1.0)
                            chance *= trophyChance.multiplicativeFactor;
                        if (chance > 1.0) chance = 1.0;
                        if (chance > 0.0 && (std::rand() % 10000) < static_cast<int>(chance * 10000.0))
                        {
                            std::ostringstream trophyKey;
                            trophyKey << "trophy:" << event.monsterCode;
                            (void)progression::AddServerCovenantInventoryItem(
                                event.playerName, covenants::catalog::kHunters, trophyKey.str(), 1);
                        }
                    }
                }
            });

    (void)huntingSubscription;
    (void)covenantSubscription;
}
}

GameplayEventHub& ServerGameplayEventHub()
{
    static GameplayEventHub hub;
    return hub;
}

void PublishMonsterDefeated(const MonsterDefeatedEvent& event)
{
    if (!event.monster.Valid())
        return;
    ServerGameplayEventHub().Publish(event);
}

void PublishMonsterKilled(const MonsterKilledEvent& event)
{
    if (!event.player.Valid() || !event.monster.Valid())
        return;

    EnsureCoreConsumersRegistered();
    ServerGameplayEventHub().Publish(event);
}
void PublishWorldEventStarted(const WorldEventStarted& event)
{
    if(event.eventKey.empty())
        return;
    ServerGameplayEventHub().Publish(event);
}

void PublishEventScoreChanged(const EventScoreChanged& event)
{
    if(event.eventKey.empty() || !event.player.Valid())
        return;
    ServerGameplayEventHub().Publish(event);
}

void PublishWorldEventEnded(const WorldEventEnded& event)
{
    if(event.eventKey.empty())
        return;
    ServerGameplayEventHub().Publish(event);
}

}
}
