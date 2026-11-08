#pragma once
// SUMMIT432A: runtime dispatcher for JSON-projected Crafting catalog 424A/425A.
// NEVER debits/grants: the 430A/431A DAT commit is still not wired.
// This quarantines the legacy SQLite recipe execution path, not SQLite's
// unrelated virtual inventories or profession progression.
#include "systems/gameplay/crafting/v2/CraftRegistry425.generated.h"
#include <cctype>
#include <string>
#include <vector>

namespace summit_craft_432 {
inline bool Code(const std::string& raw,std::string& code) {
    code.clear();
    if(raw.empty() || raw.size()>16u) return false;
    for(unsigned char c:raw) {
        if(c>='a' && c<='z')c=static_cast<unsigned char>(c-'a'+'A');
        if(!((c>='A'&&c<='Z')||(c>='0'&&c<='9')))return false;
        code.push_back(static_cast<char>(c));
    }
    return true;
}
inline bool RequestId(const std::string& request) {
    if(request.empty()||request.size()>48u)return false;
    for(unsigned char c:request)
        if(!((c>='A'&&c<='Z')||(c>='a'&&c<='z')||
             (c>='0'&&c<='9')||c=='-'||c=='_'))return false;
    return true;
}
inline std::string Status() {
    return std::string("Crafting JSON 424A/425A; receitas=")+
        std::to_string(summit_craft_425::kRecipeCount)+
        "; SQLite receitas=INATIVO; DAT craft=INATIVO; sha="+
        std::string(summit_craft_425::kSourceSHA).substr(0,12);
}
inline std::vector<std::string> List(const std::string& rawPrefix) {
    std::vector<std::string> lines;
    std::string prefix;
    if(!rawPrefix.empty()&&!Code(rawPrefix,prefix)) {
        lines.push_back("Craft list: prefixo de codigo invalido.");return lines;
    }
    for(std::size_t i=0;i<summit_craft_425::kRecipeCount;++i) {
        const summit_craft_425::Rule& r=summit_craft_425::kRecipes[i];
        const std::string code(r.code);
        if(code.compare(0,prefix.size(),prefix)!=0)continue;
        lines.push_back(code+" x"+std::to_string(r.outputCount)+
                        (r.idValid?" [JSON DRAFT]":" [IDENTIDADE PENDENTE]"));
        if(lines.size()>=18u)break;
    }
    if(lines.empty())lines.push_back("Craft list: nenhum codigo JSON para esse prefixo.");
    return lines;
}
inline std::string RefuseMake(const std::string& rawCode,const std::string& request) {
    std::string code;
    if(!Code(rawCode,code)) return "CRAFT_REJECTED: codigo de item invalido; sem consumo.";
    if(!RequestId(request)) return "CRAFT_REJECTED: requestId invalido/ausente; sem consumo.";
    const summit_craft_425::Rule* r=summit_craft_425::Find(code.c_str());
    if(!r) return "CRAFT_REJECTED: item ausente do catalogo JSON; sem consumo.";
    if(!r->idValid || !r->native)
        return "CRAFT_REJECTED "+code+": identidade nativa nao sincronizada; sem consumo.";
    if(r->size==0 || r->positive==0 || r->zero!=0 || r->bad!=0)
        return "CRAFT_REJECTED "+code+": receita JSON incompleta; 0 nao e gratis.";
    // Even fully configured recipes are disabled in 424A. No sqlite fallback.
    // Only the future durable online DAT coordinator may return a business ACK.
    return "CRAFT_REJECTED "+code+": sem commit DAT online/ACK; ingredientes intactos.";
}
} // namespace summit_craft_432
