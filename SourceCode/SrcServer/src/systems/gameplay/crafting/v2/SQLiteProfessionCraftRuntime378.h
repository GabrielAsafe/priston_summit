#pragma once
// SUMMIT 373/374/375/378: Material crafts execute entirely in SQLite.
// Outputs needing DAT/inventory slots are categorically refused before debit.
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <vector>
#include <limits>
#include "systems/persistence/database/core/Database.h"
#include "systems/gameplay/professions/v2/ProfessionRuntime.h"
#include "systems/gameplay/gathering/v2/GmDropConfig415.h" // SUMMIT433A native-backed resource classification
#include "gameplay/professions/v2/ProfessionLevelRules.h"

namespace summit_professions_378 {
inline bool KeySafe(const std::string& key) {
    if (key.empty() || key.size()>95) return false;
    for (std::size_t i=0;i<key.size();++i) {
        const char c=key[i];
        if (!((c>='a'&&c<='z')||(c>='0'&&c<='9')||c=='.'||c=='_'))return false;
    }
    return true;
}
// 406A: a replay key is valid only when it has an explicit, bounded ASCII form.
inline bool SafeRequestKey(const std::string& key) {
    if(key.empty() || key.size()>48) return false;
    for(const char c:key)
        if(!((c>='a'&&c<='z')||(c>='0'&&c<='9')||c=='_')) return false;
    return true;
}
inline bool ReadNumber(SQLConnection& db,int column,int& value) {
    return db.GetData(&value,sizeof(value),column,PARAMTYPE_Integer)!=FALSE;
}
inline bool ReadKey(SQLConnection& db,int column,std::string& value) {
    char text[128]={0};
    if(!db.GetData(text,sizeof(text),column,PARAMTYPE_String))return false;
    value=text;return true;
}
// SUMMIT433A: JSON 424A is the ONLY authority for recipes and costs.
// SQLite is NOT allowed to debit native resources or execute any legacy recipe.
// Retain signatures for link/source compatibility, but fail closed if invoked.
inline bool IsNativeResource433(const std::string& resource) {
    for(const auto& item : summit_gm415::NativeItems)
        if(resource==item.resource || resource==item.code) return true;
    return false;
}
inline bool WriteBalance(SQLConnection&,const char[256],const std::string&,int) {
    return false; // Legacy craft balance mutation permanently retired.
}
inline std::string RecipeSqliteStatus() {
    return "Receitas SQL desativadas. Fonte unica: JSON 424A/425A.";
}
inline bool ListRecipes(const std::string&,std::vector<std::string>& output) {
    output.clear();return false; // No recipe lookup in SQLite.
}
inline std::string CraftMaterial(const std::string&,const std::string&,
                                 const std::string& =std::string()) {
    return "CRAFT_REJECTED: motor SQL legacy desativado; usa o JSON 424A; sem consumo.";
}

// 407A: direct server-owned snapshot for the authenticated player's UI.
// Read only: this path never spends materials, grants items or writes a DAT.
inline bool BuildLiveSnapshot407(const std::string& playerKey,
                                 const std::string& requestKey,
                                 std::vector<std::string>& frames) {
    frames.clear();
    if(playerKey.empty() || playerKey.size()>255 || !SafeRequestKey(requestKey))return false;
    std::shared_ptr<SQLConnection> db=SQLConnection::GetConnection(DATABASEID_ServerDB);
    if(!db || !db->Open())return false;
    char player[256]={0};std::strncpy(player,playerKey.c_str(),sizeof(player)-1);
    const std::string prefix="@S407|"+requestKey+"|";
    std::vector<std::string> rows;
    bool ok=db->Prepare("SELECT ResourceKey,Quantity FROM GatheringPlayerInventory "
                        "WHERE PlayerKey=? AND Quantity>0 ORDER BY ResourceKey LIMIT 97") &&
            db->BindInputParameter(player,1,PARAMTYPE_String) && db->Execute(false);
    while(ok && db->NextRow()) {
        std::string resource;int quantity=0;
        ok=ReadKey(*db,1,resource) && ReadNumber(*db,2,quantity);
        if(!ok || resource.empty() || resource.size()>127 || quantity<=0) {ok=false;break;}
        // Native resources, including uppercase CI codes, are authoritative in DAT.
        if(IsNativeResource433(resource))continue;
        if(!KeySafe(resource) || rows.size()>=96) {ok=false;break;}
        rows.push_back(prefix+"B|"+resource+"|"+std::to_string(quantity));
    }
    if(ok)ok=db->Prepare("SELECT ProfessionType,Level,Experience FROM ProfessionProgress "
                         "WHERE PlayerKey=? AND ProfessionType BETWEEN 1 AND 7 ORDER BY ProfessionType") &&
              db->BindInputParameter(player,1,PARAMTYPE_String) && db->Execute(false);
    bool seen[8]={false};
    while(ok && db->NextRow()) {
        int type=0,level=0;long long experience=0;
        ok=ReadNumber(*db,1,type) && ReadNumber(*db,2,level) &&
           db->GetData(&experience,sizeof(experience),3,PARAMTYPE_Int64)!=FALSE;
        if(!ok||type<1||type>7||seen[type]||level<0||level>999||experience<0) {ok=false;break;}
        seen[type]=true;
        rows.push_back(prefix+"P|"+std::to_string(type)+"|"+
                       std::to_string(level)+"|"+std::to_string(experience));
    }
    db->Close();
    if(!ok)return false;
    // Emit a complete snapshot; a client only commits it after matching the
    // count in the end frame (partial network deliveries are never displayed).
    const std::size_t count=rows.size();
    frames.swap(rows);
    frames.push_back(prefix+"E|"+std::to_string(count));
    return true;
}
}
