#pragma once
// SUMMIT439A: stable lookup for the existing seven-profession progression.
// Uses the SAME XP thresholds as ProfessionLevelRules.h; no duplicate balances.
#include <algorithm>
#include <cstdint>
#include <limits>
#include <string>
#include "gameplay/professions/v2/ProfessionTypes.h"
#include "gameplay/professions/v2/ProfessionLevelRules.h"

namespace summit_prof439 {
using gameplay::professions::ProfessionType;
struct View {
    const char* name;
    int level;
    std::uint64_t experience;
    std::uint64_t nextLevelTotalXP;
    bool maxLevel;
};
inline const char* Name(ProfessionType p) {
    switch(p) {
    case ProfessionType::Arcanist:return "Arcanist";
    case ProfessionType::Blacksmith:return "Blacksmith";
    case ProfessionType::Lumberjack:return "Lumberjack";
    case ProfessionType::Miner:return "Miner";
    case ProfessionType::Artisan:return "Artisan";
    case ProfessionType::Fisher:return "Fisher";
    case ProfessionType::Hunter:return "Hunter";
    default:return nullptr;
    }
}
// level n threshold is 5*n*(n+1), as checked against the source JSON (0..50).
inline std::uint64_t Threshold(int level) {
    if(level<=0)return 0;
    if(level>50)level=50;
    return std::uint64_t(5)*std::uint64_t(level)*std::uint64_t(level+1);
}
inline View For(ProfessionType p,std::uint64_t xp) {
    const char* name=Name(p);
    const int level=gameplay::professions::ProfessionLevelForExperience(xp);
    return {name?name:"Unknown",level,xp,level>=50?Threshold(50):Threshold(level+1),level>=50};
}
inline bool Unlocked(const View& player,int requiredLevel) {
    return requiredLevel>=0&&requiredLevel<=50&&player.level>=requiredLevel;
}
// XP is granted ONLY after a confirmed grant in the physical node adapter.
// Crafting reward/XP must be emitted after durable DAT receipt (not on request).
inline ProfessionType ForGrantedNativeCI(const std::string& code) {
    if(code=="CI168")return ProfessionType::Fisher;
    if(code=="CI167")return ProfessionType::Arcanist; // herb gathering
    if(code=="CI165"||code=="CI113"||code=="CI114"||code=="CI115"||
       code=="CI116"||code=="CI117"||code=="CI118"||code=="CI119"||
       code=="CI120"||code=="CI121"||code=="CI122")
       return ProfessionType::Lumberjack;
    bool validCI=(code.size()==5&&code[0]=='C'&&code[1]=='I');
    if(validCI)for(int i=2;i<5;++i)if(code[i]<'0'||code[i]>'9')validCI=false;
    if(code=="CI166"||(validCI&&
       ((code>="CI101"&&code<="CI112")||(code>="CI123"&&code<="CI130"))))
       return ProfessionType::Miner;
    return ProfessionType::None; // no invented blacksmith/artisan/hunter XP
}
} // namespace summit_prof439
