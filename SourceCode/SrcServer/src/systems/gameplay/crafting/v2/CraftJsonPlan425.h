#pragma once
// SUMMIT425A: universal 424A recipe metadata + authenticated backpack PREVIEW.
// This is NOT a craft, grant, debit, permission to write DAT or business ACK.
#include "systems/gameplay/crafting/v2/PhysicalCraftReadiness423.h"
#include "systems/gameplay/crafting/v2/CraftRegistry425.generated.h"
#include "systems/gameplay/crafting/v2/CraftPhysicalSelection425.h"
#include "systems/gameplay/crafting/v2/CraftRecipeActivation444.generated.h" // SUMMIT444A
#include <algorithm>
#include <set>
#include <string>
#include <vector>

namespace summit_craft_425 {
inline std::vector<std::string> Plan(rsPLAYINFO* player,const std::string& rawCode) {
    std::vector<std::string> output;
    if(!player || rawCode.empty() || rawCode.size()>16u) {
        output.push_back("Craft plan: jogador ou codigo invalido."); return output;
    }
    std::string code(rawCode);
    for(char& c:code) {
        if(c>='a'&&c<='z') c=static_cast<char>(c-'a'+'A');
        if(!((c>='A'&&c<='Z')||(c>='0'&&c<='9'))) {
            output.push_back("Craft plan: codigo contem caracteres invalidos.");return output;
        }
    }
    const Rule* rule=Find(code.c_str());
    if(!rule) {output.push_back("Craft plan: item nao existe no catalogo 424A compilado.");return output;}
    output.push_back("Craft plan "+code+" x"+std::to_string(rule->outputCount)+
      (code=="WA103" ? " [FABRICO DAT]" : " [SEM FABRICO]"));
    // SUMMIT444A: authoritative compiled flags from the same 424A JSON as the registry.
    // A complete ingredient list is NOT permission to craft or to mutate DAT.
    const auto* activation444=summit_craft_444::Find(code.c_str());
    if(!activation444) {
        output.push_back("Contrato JSON/registry inconsistente. FABRICO BLOQUEADO.");return output;
    }
    output.push_back(activation444->complete ?
        "JSON 424A: ingredientes configurados." : "JSON 424A: ingredientes por configurar.");
    output.push_back(activation444->jsonEnabled ?
        "JSON 424A: receita marcada ativa." : "JSON 424A: receita DESATIVADA.");
    output.push_back(code=="WA103" ?
        "WA103: fabrico DAT transacional disponível quando o servidor estiver configurado." :
        "DAT: outras receitas continuam BLOQUEADAS.");
    output.push_back("Aviso: contagem abaixo e de instancias legacy, nao unidades verificadas de stacks CI.");
    if(!rule->idValid) {
        output.push_back("Codigo nao esta sincronizado nos perfis cliente e servidor.");return output;
    }
    if(rule->size==0 || rule->positive==0) {
        output.push_back("Receita sem materiais definidos. Configurar JSON 424A; zero nao e gratis.");
        return output;
    }
    if(rule->bad>0) {
        output.push_back("Ingredientes sem identidade nativa valida nos dois perfis.");return output;
    }
    if(rule->zero>0) {
        output.push_back("Receita incompleta: quantidade zero ainda precisa de configuracao.");
        for(std::size_t i=rule->first;i<rule->first+rule->size;++i)
            if(kIngredients[i].count==0)
                output.push_back(std::string(kIngredients[i].code)+" = 0 [PENDENTE]");
        return output;
    }
    // Even a complete recipe is always READ ONLY. Use a fresh, authenticated,
    // generation-bound inventory snapshot (the same contract as 423A).
    const auto& registry=gameplay::player::legacy::ServerLegacyOnlinePlayerRegistry();
    const auto session=registry.HandleOf(player);
    if(!session.Valid() || registry.Resolve(session)!=player) {
        output.push_back("Sessao mudou; sem inventario.");return output;
    }
    gameplay::inventory::InventorySnapshot snapshot;
    if(!gameplay::inventory::legacy::ServerLegacyInventoryRuntime()
           .TryGetSnapshot(session.id,snapshot) || registry.Resolve(session)!=player) {
        output.push_back("Inventario nativo indisponivel.");return output;
    }
    std::vector<summit_craft_423::Ingredient> requirements;
    requirements.reserve(rule->size);
    std::set<std::uint32_t> codes;
    for(std::size_t i=rule->first;i<rule->first+rule->size;++i) {
        const Ingredient& ing=kIngredients[i];
        if(ing.native==0 || ing.count<=0 || !codes.insert(ing.native).second) {
            output.push_back("Ingredientes ambiguos; sem consumo.");return output;
        }
        requirements.emplace_back(ing.code,ing.native,ing.count);
    }
    std::vector<summit_craft_423::Instance> instances;
    for(const auto& slot:snapshot.slots) {
        if(codes.count(slot.item.item.value))
            instances.emplace_back(slot.index,slot.item.item.value,slot.item.key,slot.item.checksum);
    }
    const auto preview=summit_craft_423::Preview(requirements,instances);
    if(preview.status==summit_craft_423::Status::InvalidRecipe ||
       preview.status==summit_craft_423::Status::AmbiguousInventory) {
        output.push_back("Inventario invalido/ambiguo. Craft impedido.");return output;
    }
    for(const auto& ingredient:preview.lines)
        output.push_back(ingredient.resource+" "+std::to_string(ingredient.available)+"/"+
                         std::to_string(ingredient.required)+" [MOCHILA]");
    const auto selection=SelectInstances(requirements,instances);
    if((preview.status==summit_craft_423::Status::Ready && selection.status!=SelectionStatus::Ready) ||
       (preview.status==summit_craft_423::Status::Missing && selection.status!=SelectionStatus::Missing)) {
        output.push_back("Divergencia no planeamento de instancias; sem consumo.");return output;
    }
    output.push_back(code=="WA103" ?
       "WA103: vista por instancia; quantidades assinadas verificadas no commit DAT." :
       (preview.status==summit_craft_423::Status::Ready ?
       "Ingredientes disponiveis. FABRICO DAT AINDA BLOQUEADO." :
       "Faltam materiais. FABRICO DAT AINDA BLOQUEADO."));
    return output;
}
} // namespace summit_craft_425
