#pragma once
// SUMMIT467A: SQLite idempotent XP for the authenticated, saved physical craft.
// This service NEVER grants an inventory item and NEVER decodes/writes DAT.
// Calling GrantVerified is permitted only from the server's ValidatedSave path
// AFTER SavedResultMatches has confirmed the signed output and removed inputs.
#include "gameplay/crafting/v2/CraftXPRecipe467.generated.h"
#include "systems/persistence/database/core/Database.h"
#include "gameplay/professions/v2/ProfessionLevelRules.h"
#include "systems/gameplay/professions/v2/ProfessionRuntime.h"
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <cstdio>

namespace summit467xp {
inline bool SafeText(const char* text,std::size_t max){
    if(!text||!text[0])return false;
    std::size_t length=0;
    for(;text[length] && length<=max;++length){
        const unsigned char c=static_cast<unsigned char>(text[length]);
        if(c<32 || c>126)return false;
    }
    return length>0 && length<=max;
}
inline bool SafeRequest(const char* key){
    if(!key)return false;
    std::size_t size=0;
    for(;key[size] && size<=48;++size){
        const char c=key[size];
        if(!((c>='A'&&c<='Z')||(c>='a'&&c<='z')||
             (c>='0'&&c<='9')||c=='_'))return false;
    }
    return size>0&&size<=48;
}
inline bool DbReady(const std::shared_ptr<SQLConnection>& db){return db && db->Open();}
inline bool BindKey(SQLConnection& db,char* player,char* key){
    return db.BindInputParameter(player,1,PARAMTYPE_String) &&
           db.BindInputParameter(key,2,PARAMTYPE_String);
}
inline bool Exists(SQLConnection& db,char* player,char* request,bool& result){
    result=false;
    if(!db.Prepare("SELECT 1 FROM ProfessionPhysicalCraftXP467 "
                   "WHERE PlayerKey=? AND RequestKey=?") ||
       !BindKey(db,player,request) || !db.Execute(false))return false;
    result=db.NextRow()!=FALSE;
    return true;
}
inline bool ReserveIntent(const char* playerName,const char* requestKey,const char* recipeCode){
    const Rule* rule=Find(recipeCode);
    if(!rule || !SafeText(playerName,255) || !SafeRequest(requestKey))return false;
    char player[256]={0},request[49]={0},recipe[17]={0};
    std::strncpy(player,playerName,sizeof(player)-1);
    std::strncpy(request,requestKey,sizeof(request)-1);
    std::strncpy(recipe,recipeCode,sizeof(recipe)-1);
    std::shared_ptr<SQLConnection> db=SQLConnection::GetConnection(DATABASEID_ServerDB);
    if(!DbReady(db))return false;
    if(!db->BeginTransaction()){db->Close();return false;}
    bool existed=false;
    bool ok=Exists(*db,player,request,existed) && !existed;
    int type=rule->profession,xp=rule->xp;
    if(ok)ok=db->Prepare(
       "INSERT INTO ProfessionPhysicalCraftXP467"
       "(PlayerKey,RequestKey,RecipeCode,ProfessionType,XPAward,State)"
       " VALUES(?,?,?,?,?,0)") &&
       db->BindInputParameter(player,1,PARAMTYPE_String) &&
       db->BindInputParameter(request,2,PARAMTYPE_String) &&
       db->BindInputParameter(recipe,3,PARAMTYPE_String) &&
       db->BindInputParameter(&type,4,PARAMTYPE_Integer) &&
       db->BindInputParameter(&xp,5,PARAMTYPE_Integer) && db->Execute(false);
    if(ok)ok=db->CommitTransaction();
    if(!ok)db->RollbackTransaction();
    db->Close();return ok;
}

// Called only with proof of server-validated save: this API does NOT accept
// a client-provided XP award, profession, item code, or completion flag.
inline bool GrantVerified(const char* playerName,const char* requestKey){
    if(!SafeText(playerName,255)||!SafeRequest(requestKey))return false;
    char player[256]={0},request[49]={0};
    std::strncpy(player,playerName,sizeof(player)-1);
    std::strncpy(request,requestKey,sizeof(request)-1);
    std::shared_ptr<SQLConnection> db=SQLConnection::GetConnection(DATABASEID_ServerDB);
    if(!DbReady(db))return false;
    if(!db->BeginTransaction()){db->Close();return false;}
    bool ok=db->Prepare("SELECT ProfessionType,XPAward,State "
           "FROM ProfessionPhysicalCraftXP467 WHERE PlayerKey=? AND RequestKey=?") &&
           BindKey(*db,player,request) && db->Execute(false) && db->NextRow();
    int type=0,xp=0,state=-1;
    if(ok)ok=db->GetData(&type,sizeof(type),1,PARAMTYPE_Integer)!=FALSE &&
              db->GetData(&xp,sizeof(xp),2,PARAMTYPE_Integer)!=FALSE &&
              db->GetData(&state,sizeof(state),3,PARAMTYPE_Integer)!=FALSE;
    if(!ok || type<1 || type>7 || xp<0 || xp>1000 || (state!=0&&state!=1)){
        db->RollbackTransaction();db->Close();return false;
    }
    if(state==1){
        // Replay never grants more XP. SQL transaction still owns the read.
        db->RollbackTransaction();db->Close();return true;
    }
    // Use the exact profession rules already used by SQLite craft 378.
    int oldLevel=0;long long oldXp=0;
    ok=db->Prepare("SELECT Level,Experience FROM ProfessionProgress "
                   "WHERE PlayerKey=? AND ProfessionType=?") &&
       db->BindInputParameter(player,1,PARAMTYPE_String) &&
       db->BindInputParameter(&type,2,PARAMTYPE_Integer) && db->Execute(false);
    if(ok && db->NextRow()){
        ok=db->GetData(&oldLevel,sizeof(oldLevel),1,PARAMTYPE_Integer)!=FALSE &&
            db->GetData(&oldXp,sizeof(oldXp),2,PARAMTYPE_Int64)!=FALSE;
    }
    if(!ok || oldLevel<0 || oldLevel>999 || oldXp<0 || oldXp>0x7fffffffffffffffLL-xp){
        db->RollbackTransaction();db->Close();return false;
    }
    long long total=oldXp+xp;
    int level=gameplay::professions::ProfessionLevelForExperience(static_cast<std::uint64_t>(total));
    if(level<oldLevel)level=oldLevel;
    if(level>999){db->RollbackTransaction();db->Close();return false;}
    ok=db->Prepare("INSERT INTO ProfessionProgress(PlayerKey,ProfessionType,Level,Experience,UpdatedAt) "
      "VALUES(?,?,?,?,datetime('now')) ON CONFLICT(PlayerKey,ProfessionType) DO UPDATE SET "
      "Level=excluded.Level,Experience=excluded.Experience,UpdatedAt=excluded.UpdatedAt") &&
      db->BindInputParameter(player,1,PARAMTYPE_String) &&
      db->BindInputParameter(&type,2,PARAMTYPE_Integer) &&
      db->BindInputParameter(&level,3,PARAMTYPE_Integer) &&
      db->BindInputParameter(&total,4,PARAMTYPE_Int64) && db->Execute(false);
    if(ok)ok=db->Prepare("UPDATE ProfessionPhysicalCraftXP467 "
                         "SET State=1,GrantedAt=datetime('now') "
                         "WHERE PlayerKey=? AND RequestKey=? AND State=0") &&
          BindKey(*db,player,request) && db->Execute(false);
    if(ok)ok=db->CommitTransaction();
    if(!ok)db->RollbackTransaction();
    db->Close();
    if(ok)gameplay::professions::ServerProfessionService().ClearPlayer(std::string(player));
    return ok;
}
} // summit467xp
