#pragma once
// SUMMIT 351C / canonical V2 persistent kill tracking.
// These entries are independent from 350Z character records and ordinary QuestDB tables.
// No reward grant is performed here. NPC return / payout must be validated separately.
#include <cstdint>
#include <ctime>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <string>
#include <vector>
#include "gameplay/quests/v2/QuestCanonicalKillRules351C.h"
#include "gameplay/quests/v2/QuestCanonicalRegistry351B.h"
#include "systems/persistence/database/core/Database.h"
namespace gameplay { namespace quests { namespace canonical {
inline bool Enabled351C() {
 // Opt-in until quests' item and XP rewards are mapped to authoritative server IDs.
 const char* s=std::getenv("SUMMIT_ENABLE_QUEST_PROGRESS_351C");
 return s && std::strcmp(s,"1")==0;
}
inline int Period351C(int kind, std::time_t t) {
 const std::int64_t day=static_cast<std::int64_t>(t)/86400;
 // UTC day, or 1970-01-05-based Monday week. Progression not routed here.
 return kind==kWeeklyKind351B ? static_cast<int>((day+3)/7+1000000) : static_cast<int>(day);
}
inline bool Key351C(const std::string& player) {return !player.empty() && player.size()<32u;}
inline bool EnsureSchema351C(const std::shared_ptr<SQLConnection>& db) {
 return db->Prepare("CREATE TABLE IF NOT EXISTS [QuestDB__SummitV2QuestKill351C] ("
   "PlayerName TEXT NOT NULL, QuestID INTEGER NOT NULL, Period INTEGER NOT NULL, "
   "ObjectiveIndex INTEGER NOT NULL, Target TEXT NOT NULL, Needed INTEGER NOT NULL, "
   "Killed INTEGER NOT NULL DEFAULT 0, PRIMARY KEY(PlayerName,QuestID,Period,ObjectiveIndex))")
   && db->Execute(false);
}
struct KillStatus351C { bool started; bool ready; std::vector<int> counts; KillStatus351C():started(false),ready(false),counts(){} };
inline bool ReadKills351C(const std::string& player, const Entry351B& entry,
                          std::time_t now, KillStatus351C& result) {
 if(!Key351C(player))return false;
 const KillObjective351C* base=0; const unsigned int n=ObjectivesFor351C(entry.id,base);
 if(!n)return false;
 std::shared_ptr<SQLConnection> db=SQLConnection::GetConnection(DATABASEID_QuestDB);
 if(!db || !db->Open())return false;
 if(!EnsureSchema351C(db)){db->Close();return false;}
 int quest=static_cast<int>(entry.id),period=Period351C(Kind351B(entry),now);
 char key[32]={};std::memcpy(key,player.data(),player.size());
 const bool ok=db->Prepare("SELECT ObjectiveIndex,Killed,Needed FROM [QuestDB__SummitV2QuestKill351C] "
         "WHERE PlayerName=? AND QuestID=? AND Period=? ORDER BY ObjectiveIndex")
   && db->BindInputParameter(key,1,PARAMTYPE_String)
   && db->BindInputParameter(&quest,2,PARAMTYPE_Integer)
   && db->BindInputParameter(&period,3,PARAMTYPE_Integer)
   && db->Execute(false);
 if(!ok){db->Close();return false;}
 KillStatus351C r; r.counts.resize(n,-1);unsigned int total=0;bool ready=true;
 while(db->NextRow()){
  int idx=0,k=0,needed=0;
  if(!db->GetData(&idx,4,1,PARAMTYPE_Integer) ||
     !db->GetData(&k,4,2,PARAMTYPE_Integer) ||
     !db->GetData(&needed,4,3,PARAMTYPE_Integer) ||
     idx<0 || static_cast<unsigned int>(idx)>=n || k<0 || k>needed || needed!=base[idx].needed || r.counts[idx]>=0){db->Close();return false;}
  r.counts[idx]=k;ready=ready && k>=needed;++total;
 }
 db->Close();
 if(total && total!=n)return false; // Never treat a half-written admission as valid.
 r.started=total==n;r.ready=r.started && ready;result=r;return true;
}
inline bool StartKills351C(const std::string& player, const Entry351B& entry,
                           int level, int job, std::time_t now) {
 if(!Enabled351C() || !Key351C(player) || level<entry.minLevel ||
    (entry.maxLevel>0 && level>entry.maxLevel))return false;
 // Two starting daily quests are faction-restricted (job 9=Assassin, 10=Shaman).
 if(entry.id>=0xE3510018u && entry.id<=0xE351001Bu) {
  const bool morion=(job>=5&&job<=8)||job==10;
  const bool tempskron=(job>=1&&job<=4)||job==9;
  if((entry.id==0xE3510018u||entry.id==0xE3510019u) ? !morion : !tempskron)return false;
 }
 const KillObjective351C* base=0;const unsigned int n=ObjectivesFor351C(entry.id,base);
 if(!n)return false;
 KillStatus351C current;
 if(!ReadKills351C(player,entry,now,current) || current.started)return false;
 std::shared_ptr<SQLConnection> db=SQLConnection::GetConnection(DATABASEID_QuestDB);
 if(!db||!db->Open())return false;
 if(!EnsureSchema351C(db)||!db->BeginTransaction()){db->Close();return false;}
 char key[32]={};std::memcpy(key,player.data(),player.size());
 int quest=static_cast<int>(entry.id),period=Period351C(Kind351B(entry),now);
 bool ok=true;
 for(unsigned int slot=0;slot<n && ok;++slot){
  int index=static_cast<int>(slot),need=base[slot].needed;
  char target[64]={};if(std::strlen(base[slot].target)>=sizeof(target)){ok=false;break;}
  std::strcpy(target,base[slot].target);
  ok=db->Prepare("INSERT INTO [QuestDB__SummitV2QuestKill351C] "
    "(PlayerName,QuestID,Period,ObjectiveIndex,Target,Needed,Killed) VALUES (?,?,?,?,?,?,0)")
   &&db->BindInputParameter(key,1,PARAMTYPE_String)
   &&db->BindInputParameter(&quest,2,PARAMTYPE_Integer)
   &&db->BindInputParameter(&period,3,PARAMTYPE_Integer)
   &&db->BindInputParameter(&index,4,PARAMTYPE_Integer)
   &&db->BindInputParameter(target,5,PARAMTYPE_String)
   &&db->BindInputParameter(&need,6,PARAMTYPE_Integer)
   &&db->Execute(false);
 }
 ok=ok&&db->CommitTransaction();
 if(!ok)db->RollbackTransaction();
 db->Close();return ok;
}
inline bool CancelKills351C(const std::string& player,const Entry351B& entry,std::time_t now) {
 if(!Enabled351C()||!Key351C(player))return false;
 KillStatus351C s; if(!ReadKills351C(player,entry,now,s)||!s.started)return false;
 std::shared_ptr<SQLConnection> db=SQLConnection::GetConnection(DATABASEID_QuestDB);
 if(!db||!db->Open())return false;
 char key[32]={};std::memcpy(key,player.data(),player.size());
 int quest=static_cast<int>(entry.id),period=Period351C(Kind351B(entry),now);
 const bool ok=db->Prepare("DELETE FROM [QuestDB__SummitV2QuestKill351C] WHERE PlayerName=? AND QuestID=? AND Period=?")
   &&db->BindInputParameter(key,1,PARAMTYPE_String)
   &&db->BindInputParameter(&quest,2,PARAMTYPE_Integer)
   &&db->BindInputParameter(&period,3,PARAMTYPE_Integer)
   &&db->Execute(false);
 db->Close();return ok;
}
struct KillIncrement351C {int quest,period,slot;};
inline bool RecordKills351C(const std::string& player,const std::string& monster,
                            bool boss,bool partyCredit,std::time_t now) {
 if(!Enabled351C()||!Key351C(player)||monster.empty()||partyCredit)return false;
 std::shared_ptr<SQLConnection> db=SQLConnection::GetConnection(DATABASEID_QuestDB);
 if(!db||!db->Open())return false;
 char key[32]={};std::memcpy(key,player.data(),player.size());
 int daily=Period351C(kDailyKind351B,now),weekly=Period351C(kWeeklyKind351B,now);
 const bool ok=db->Prepare("SELECT QuestID,Period,ObjectiveIndex,Target,Needed,Killed "
  "FROM [QuestDB__SummitV2QuestKill351C] WHERE PlayerName=? AND Period IN (?,?) AND Killed<Needed")
   &&db->BindInputParameter(key,1,PARAMTYPE_String)
   &&db->BindInputParameter(&daily,2,PARAMTYPE_Integer)
   &&db->BindInputParameter(&weekly,3,PARAMTYPE_Integer)
   &&db->Execute(false);
 if(!ok){db->Close();return false;}
 std::vector<KillIncrement351C> updates;
 while(db->NextRow()){
  KillIncrement351C k={0,0,0};char target[64]={};int need=0,killed=0;
  if(!db->GetData(&k.quest,4,1,PARAMTYPE_Integer) ||
     !db->GetData(&k.period,4,2,PARAMTYPE_Integer) ||
     !db->GetData(&k.slot,4,3,PARAMTYPE_Integer) ||
     !db->GetData(target,64,4,PARAMTYPE_String) ||
     !db->GetData(&need,4,5,PARAMTYPE_Integer) ||
     !db->GetData(&killed,4,6,PARAMTYPE_Integer)) {db->Close();return false;}
  const Entry351B* entry=FindCanonicalQuest351B(QuestId(static_cast<std::uint32_t>(k.quest)));
  if(!entry || k.period!=Period351C(Kind351B(*entry),now))continue;
  const KillObjective351C* base=0; const unsigned int n=ObjectivesFor351C(entry->id,base);
  if(k.slot<0||static_cast<unsigned int>(k.slot)>=n || need!=base[k.slot].needed ||
     std::strcmp(target,base[k.slot].target)!=0)continue;
  if(MatchesKill351C(base[k.slot],monster.c_str(),boss))updates.push_back(k);
 }
 if(updates.empty()){db->Close();return true;}
 if(!db->BeginTransaction()){db->Close();return false;}
 bool wrote=true;
 for(std::size_t i=0;i<updates.size()&&wrote;++i){
  KillIncrement351C& k=updates[i];
  wrote=db->Prepare("UPDATE [QuestDB__SummitV2QuestKill351C] SET Killed=Killed+1 "
     "WHERE PlayerName=? AND QuestID=? AND Period=? AND ObjectiveIndex=? AND Killed<Needed")
   &&db->BindInputParameter(key,1,PARAMTYPE_String)
   &&db->BindInputParameter(&k.quest,2,PARAMTYPE_Integer)
   &&db->BindInputParameter(&k.period,3,PARAMTYPE_Integer)
   &&db->BindInputParameter(&k.slot,4,PARAMTYPE_Integer)
   &&db->Execute(false);
 }
 if(wrote)wrote=db->CommitTransaction();
 if(!wrote)db->RollbackTransaction();
 db->Close();return wrote;
}
}}}
