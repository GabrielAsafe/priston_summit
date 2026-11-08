#pragma once
// SUMMIT423A: authenticated, READ ONLY preflight. No SQL UPDATE, DAT or item mutations.
// Included from the chat ingress after the legacy server player definitions.
#include "systems/gameplay/crafting/v2/PhysicalCraftInventoryPlan423.h"
#include "systems/gameplay/crafting/v2/SQLiteProfessionCraftRuntime378.h"
#include "systems/gameplay/gathering/v2/GmDropConfig415.h"
#include "systems/gameplay/inventory/v2/legacy/LegacyItemCatalogAdapter.h"
#include "systems/inventory/v2/legacy/LegacyInventoryRuntime.h"
#include "systems/gameplay/player/v2/legacy/LegacyOnlinePlayerRegistry.h"
#include <set>
#include <string>
#include <vector>

namespace summit_craft_423 {
inline std::vector<std::string> PhysicalCraftReadiness(rsPLAYINFO* player,
                                                      const std::string& recipe) {
    std::vector<std::string> out;
    if(!player || !summit_professions_378::KeySafe(recipe)) {
        out.push_back("Craft check: receita/sessao invalida."); return out;
    }
    std::shared_ptr<SQLConnection> db=SQLConnection::GetConnection(DATABASEID_ServerDB);
    if(!db || !db->Open()) {
        out.push_back("Craft check: SQLite indisponivel.");return out;
    }
    char key[96]={};std::strncpy(key,recipe.c_str(),sizeof(key)-1);
    int profession=0, minLevel=0, physical=0, enabled=0;
    bool ok=db->Prepare("SELECT ProfessionType,MinLevel,Physical,Enabled "
                        "FROM ProfessionRecipe378 WHERE RecipeKey=?") &&
             db->BindInputParameter(key,1,PARAMTYPE_String) && db->Execute(false) &&
             db->NextRow() && summit_professions_378::ReadNumber(*db,1,profession) &&
             summit_professions_378::ReadNumber(*db,2,minLevel) &&
             summit_professions_378::ReadNumber(*db,3,physical) &&
             summit_professions_378::ReadNumber(*db,4,enabled);
    if(!ok) {
        db->Close();out.push_back("Craft check: receita nao encontrada na BD ativa.");return out;
    }
    if(!enabled || physical!=1 || profession<1 || profession>7 || minLevel<0 || minLevel>50) {
        db->Close();out.push_back("Craft check: receita nao e DAT fisica ativa.");return out;
    }
    std::vector<Ingredient> required;
    ok=db->Prepare("SELECT ResourceKey,Amount FROM ProfessionIngredient378 "
                   "WHERE RecipeKey=? ORDER BY ResourceKey") &&
       db->BindInputParameter(key,1,PARAMTYPE_String) && db->Execute(false);
    while(ok && db->NextRow()) {
        std::string resource; int count=0;
        ok=summit_professions_378::ReadKey(*db,1,resource) &&
           summit_professions_378::ReadNumber(*db,2,count);
        if(!ok || resource.empty() || count<1 || count>10000 || required.size()>=16u) {
            ok=false;break;
        }
        std::string native;
        for(const auto& entry:summit_gm415::NativeItems) {
            if(resource==entry.resource) {
                if(!native.empty()) {ok=false;break;}
                native=entry.code;
            }
        }
        if(!ok || native.empty()) {ok=false;break;}
        gameplay::inventory::ItemDefinition definition;
        if(!gameplay::inventory::legacy::ServerLegacyItemCatalogAdapter()
             .TryGetDefinitionByContentKey(native,definition) || !definition.id.Valid()) {
            ok=false;break;
        }
        required.push_back(Ingredient{resource,definition.id.value,count});
    }
    db->Close();
    if(!ok || required.empty()) {
        out.push_back("Craft check: ingredientes/identidade nativa nao resolvidos; sem consumo.");return out;
    }
    const auto& registry=gameplay::player::legacy::ServerLegacyOnlinePlayerRegistry();
    const auto session=registry.HandleOf(player);
    if(!session.Valid() || registry.Resolve(session)!=player) {
        out.push_back("Craft check: sessao alterada; sem consumo.");return out;
    }
    gameplay::inventory::InventorySnapshot snapshot;
    if(!gameplay::inventory::legacy::ServerLegacyInventoryRuntime()
           .TryGetSnapshot(session.id,snapshot) ||
       registry.Resolve(session)!=player) {
        out.push_back("Craft check: inventario autoritativo indisponivel; sem consumo.");return out;
    }
    std::vector<Instance> items;
    items.reserve(snapshot.slots.size());
    for(const auto& slot:snapshot.slots) {
        bool relevant=false;
        for(const auto& in:required)
            if(in.nativeCode==slot.item.item.value) {relevant=true;break;}
        if(relevant) items.push_back(Instance{slot.index,slot.item.item.value,
                                             slot.item.key,slot.item.checksum});
    }
    const Check check=Preview(required,items);
    if(check.status==Status::InvalidRecipe || check.status==Status::AmbiguousInventory) {
        out.push_back("Craft check: mochila ambigua ou invalida; sem consumo.");return out;
    }
    out.push_back("Craft check [MOCHILA REAL / SEM DEBITO] "+recipe);
    for(const auto& line:check.lines) {
        // One real item identity equals one unit; no speculative potion stacks.
        out.push_back(line.resource+" "+std::to_string(line.available)+"/"+
                      std::to_string(line.required));
    }
    out.push_back(check.status==Status::Ready ?
          "Materiais presentes (instancias). FABRICO DAT AINDA BLOQUEADO." :
          "Faltam materiais fisicos. FABRICO DAT AINDA BLOQUEADO.");
    return out;
}
} // namespace summit_craft_423
