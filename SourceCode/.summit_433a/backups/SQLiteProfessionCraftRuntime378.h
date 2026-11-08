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
inline bool WriteBalance(SQLConnection& db,const char player[256],const std::string& resource,int delta) {
    if(resource.empty()||resource.size()>127||delta==0)return false;
    char key[128]={0};std::strncpy(key,resource.c_str(),sizeof(key)-1);
    bool ok=false;
    if(delta>0) {
        ok=db.Prepare("INSERT INTO GatheringPlayerInventory(PlayerKey,ResourceKey,Quantity,UpdatedAt) "
             "VALUES(?,?,?,datetime('now')) ON CONFLICT(PlayerKey,ResourceKey) DO UPDATE SET "
             "Quantity=Quantity+excluded.Quantity,UpdatedAt=excluded.UpdatedAt "
             "WHERE Quantity>=0 AND Quantity<=2147483647-excluded.Quantity") &&
            db.BindInputParameter(const_cast<char*>(player),1,PARAMTYPE_String) &&
            db.BindInputParameter(key,2,PARAMTYPE_String) &&
            db.BindInputParameter(&delta,3,PARAMTYPE_Integer) && db.Execute(false);
    } else {
        const int required=-delta;
        ok=db.Prepare("UPDATE GatheringPlayerInventory SET Quantity=Quantity-?,UpdatedAt=datetime('now') "
               "WHERE PlayerKey=? AND ResourceKey=? AND Quantity>=?") &&
           db.BindInputParameter(const_cast<int*>(&required),1,PARAMTYPE_Integer) &&
           db.BindInputParameter(const_cast<char*>(player),2,PARAMTYPE_String) &&
           db.BindInputParameter(key,3,PARAMTYPE_String) &&
           db.BindInputParameter(const_cast<int*>(&required),4,PARAMTYPE_Integer) && db.Execute(false);
    }
    int changed=0;
    return ok && db.Prepare("SELECT changes()") && db.Execute(false) && db.NextRow() &&
           ReadNumber(db,1,changed) && changed==1;
}
// Read-only diagnostics; never repairs or modifies the SQLite schema.
inline std::string RecipeSqliteStatus() {
    std::shared_ptr<SQLConnection> db=SQLConnection::GetConnection(DATABASEID_ServerDB);
    if(!db||!db->Open())return "Craft 049: servidor SQLite indisponivel.";
    int total=0,active=0,first=0;
    const bool ok=db->Prepare(
        "SELECT COUNT(*), COALESCE(SUM(CASE WHEN Enabled=1 THEN 1 ELSE 0 END),0), "
        "COALESCE(SUM(CASE WHEN RecipeKey='artisan.refine_core_crystalline' THEN 1 ELSE 0 END),0) "
        "FROM ProfessionRecipe378") && db->Execute(false) && db->NextRow() &&
        ReadNumber(*db,1,total) && ReadNumber(*db,2,active) && ReadNumber(*db,3,first);
    db->Close();
    if(!ok)return "Craft 049: tabela de receitas indisponivel. Verificar Server.exe, migracao 049 e BD ativa.";
    if(total==0)return "Craft 049: 0 receitas na BD ativa. Verificar migracao 049, sem alterar inventario.";
    return "Craft 049: receitas="+std::to_string(total)+
           " ativas="+std::to_string(active)+
           " core_crystalline="+std::to_string(first);
}
inline bool ListRecipes(const std::string& prefix,std::vector<std::string>& output) {
    output.clear();
    std::shared_ptr<SQLConnection> db=SQLConnection::GetConnection(DATABASEID_ServerDB);
    if(!db||!db->Open())return false;
    bool ok=db->Prepare("SELECT RecipeKey,Physical,MinLevel FROM ProfessionRecipe378 "
                        "WHERE Enabled=1 ORDER BY ProfessionType,MinLevel,RecipeKey") && db->Execute(false);
    while(ok && db->NextRow()) {
        std::string name;int physical=0,level=0;
        ok=ReadKey(*db,1,name)&&ReadNumber(*db,2,physical)&&ReadNumber(*db,3,level);
        if(!ok)break;
        if(!prefix.empty() && name.find(prefix)!=0)continue;
        if(output.size()<18)output.push_back(name + (physical?" [DAT BLOQUEADO]":" [SQLITE]") +
                                               " nv." + std::to_string(level));
    }
    db->Close();return ok;
}
inline std::string CraftMaterial(const std::string& playerKey,const std::string& recipeKey,
                                 const std::string& requestKey=std::string()) {
    if(playerKey.empty()||playerKey.size()>255||!KeySafe(recipeKey))return "Receita/jogador invalido.";
    if(!requestKey.empty()&&!SafeRequestKey(requestKey))return "Identificador de pedido invalido; sem consumo.";
    char player[256]={0},name[96]={0};
    std::strncpy(player,playerKey.c_str(),sizeof(player)-1);
    std::strncpy(name,recipeKey.c_str(),sizeof(name)-1);
    std::shared_ptr<SQLConnection> db=SQLConnection::GetConnection(DATABASEID_ServerDB);
    if(!db||!db->Open()||!db->BeginTransaction())return "SQLite indisponivel; nao foram gastos materiais.";
    // 406A: on a retry, a previously committed key returns without touching
    // materials, XP or receipts. The lookup and eventual INSERT share the
    // exact same SQLite transaction; a concurrent duplicate cannot both commit.
    char request[49]={0};
    if(!requestKey.empty()) {
        std::strncpy(request,requestKey.c_str(),sizeof(request)-1);
        const bool lookup=db->Prepare("SELECT RecipeKey FROM ProfessionCraftRequest406 "
              "WHERE PlayerKey=? AND RequestKey=?") &&
              db->BindInputParameter(player,1,PARAMTYPE_String) &&
              db->BindInputParameter(request,2,PARAMTYPE_String) && db->Execute(false);
        if(!lookup) {
            db->RollbackTransaction();db->Close();
            return "Pedido protegido indisponivel: aplicar migracao 050, sem consumo.";
        }
        if(db->NextRow()) {
            std::string previous;
            const bool read=ReadKey(*db,1,previous);
            db->RollbackTransaction();db->Close();
            if(!read) return "Pedido anterior indisponivel; sem novo consumo.";
            return previous==recipeKey ? "Pedido ja concluido anteriormente; sem novo consumo." :
                 "Identificador reutilizado noutra receita; sem consumo.";
        }
    }
    bool ok=db->Prepare("SELECT ProfessionType,MinLevel,OutputKey,OutputCount,Physical,XP "
                        "FROM ProfessionRecipe378 WHERE RecipeKey=? AND Enabled=1") &&
            db->BindInputParameter(name,1,PARAMTYPE_String)&&db->Execute(false);
    int type=0,minLevel=0,outCount=0,physical=0,xp=0;
    std::string output;
    if(!ok || !db->NextRow() ||
       !ReadNumber(*db,1,type)||!ReadNumber(*db,2,minLevel)||!ReadKey(*db,3,output)||
       !ReadNumber(*db,4,outCount)||!ReadNumber(*db,5,physical)||!ReadNumber(*db,6,xp)) {
        db->RollbackTransaction();db->Close();return "Receita inexistente ou leitura SQLite falhou.";
    }
    // Fundamental invariant: never debit a material for a DAT-bound product.
    if(physical!=0) { db->RollbackTransaction();db->Close();return "ITEM FISICO BLOQUEADO: falta transacao DAT/SQLite certificada."; }
    if(type!=1 && type!=2 && type!=5) { db->RollbackTransaction();db->Close();return "Profissao invalida."; }
    if(outCount<1||outCount>100||xp<0||xp>1000||output.empty()||output.size()>127) {
        db->RollbackTransaction();db->Close();return "Receita fora dos limites seguros.";
    }
    int oldLevel=0;long long oldExp=0;
    int profType=type;
    ok=db->Prepare("SELECT Level,Experience FROM ProfessionProgress "
                   "WHERE PlayerKey=? AND ProfessionType=?") &&
       db->BindInputParameter(player,1,PARAMTYPE_String) &&
       db->BindInputParameter(&profType,2,PARAMTYPE_Integer) && db->Execute(false);
    if(ok && db->NextRow())
        ok=ReadNumber(*db,1,oldLevel) &&
           db->GetData(&oldExp,sizeof(oldExp),2,PARAMTYPE_Int64)!=FALSE;
    if(!ok || oldLevel<0 || oldExp<0 || oldExp>0x7fffffffffffffffLL-xp) {
        db->RollbackTransaction();db->Close();return "Progresso invalido; nao foram gastos materiais.";
    }
    if(oldLevel<minLevel) { db->RollbackTransaction();db->Close();return "Nivel da profissao insuficiente."; }
    std::vector<std::pair<std::string,int> > ingredients;
    ok=db->Prepare("SELECT ResourceKey,Amount FROM ProfessionIngredient378 "
                   "WHERE RecipeKey=? ORDER BY ResourceKey") &&
       db->BindInputParameter(name,1,PARAMTYPE_String)&&db->Execute(false);
    while(ok && db->NextRow()) {
        std::string key;int count=0;
        ok=ReadKey(*db,1,key)&&ReadNumber(*db,2,count);
        if(!ok || key.empty() || key.size()>127 || count<=0 || count>10000) {ok=false;break;}
        ingredients.push_back(std::make_pair(key,count));
        if(ingredients.size()>16) {ok=false;break;}
    }
    if(ingredients.empty())ok=false;
    for(std::size_t i=0;ok && i<ingredients.size();++i)
        ok=WriteBalance(*db,player,ingredients[i].first,-ingredients[i].second);
    if(ok)ok=WriteBalance(*db,player,output,outCount);
    if(ok) {
        int state=0; // receipt is SQLite-only. Never a physical-grant receipt.
        (void)state;
        ok=db->Prepare("INSERT INTO ProfessionCraftReceipt378(PlayerKey,RecipeKey,ResultKey,ResultAmount,State) "
                       "VALUES(?,?,?,?,'SQLITE_COMMITTED')") &&
            db->BindInputParameter(player,1,PARAMTYPE_String) &&
            db->BindInputParameter(name,2,PARAMTYPE_String);
        char out[128]={0};std::strncpy(out,output.c_str(),sizeof(out)-1);
        ok=ok && db->BindInputParameter(out,3,PARAMTYPE_String) &&
           db->BindInputParameter(&outCount,4,PARAMTYPE_Integer) && db->Execute(false);
    }
    if(ok) {
        long long newExp=oldExp+xp;
        int newLevel=gameplay::professions::ProfessionLevelForExperience(static_cast<std::uint64_t>(newExp));
        if(newLevel<oldLevel)newLevel=oldLevel;
        ok=db->Prepare("INSERT INTO ProfessionProgress(PlayerKey,ProfessionType,Level,Experience,UpdatedAt) "
          "VALUES(?,?,?,?,datetime('now')) ON CONFLICT(PlayerKey,ProfessionType) DO UPDATE SET "
          "Level=excluded.Level,Experience=excluded.Experience,UpdatedAt=excluded.UpdatedAt") &&
          db->BindInputParameter(player,1,PARAMTYPE_String) &&
          db->BindInputParameter(&profType,2,PARAMTYPE_Integer) &&
          db->BindInputParameter(&newLevel,3,PARAMTYPE_Integer) &&
          db->BindInputParameter(&newExp,4,PARAMTYPE_Int64) && db->Execute(false);
    }
    // The replay marker is committed atomically with balances, receipt and XP.
    // If the migration is missing or this key races with another request, the
    // entire SQLite transaction rolls back. Physical outputs never reach here.
    if(ok && !requestKey.empty())
        ok=db->Prepare("INSERT INTO ProfessionCraftRequest406(PlayerKey,RequestKey,RecipeKey) "
              "VALUES(?,?,?)") &&
           db->BindInputParameter(player,1,PARAMTYPE_String) &&
           db->BindInputParameter(request,2,PARAMTYPE_String) &&
           db->BindInputParameter(name,3,PARAMTYPE_String) && db->Execute(false);
    if(ok && db->CommitTransaction()) {
        db->Close();
        gameplay::professions::ServerProfessionService().ClearPlayer(playerKey);
        return "Fabricado (SQLite): "+output+" x"+std::to_string(outCount)+". XP guardada.";
    }
    db->RollbackTransaction();db->Close();
    return "Fabricacao cancelada: ingredientes insuficientes ou erro SQLite (rollback).";
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
        if(!ok || !KeySafe(resource) || quantity<=0 || rows.size()>=96) {ok=false;break;}
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
