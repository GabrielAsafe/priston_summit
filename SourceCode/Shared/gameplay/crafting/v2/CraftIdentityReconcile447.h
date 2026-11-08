#pragma once
// SUMMIT 447A: reconcile the SAME identity scope populated by the native
// rsRECORD_DBASE::MakeRecordItemList(): both backpacks, equipped slots and
// the cursor. The legacy online ledger excludes potions and "thrown" items.
// The DAT item records need not all be ItemPosition==0. No DAT mutation here.
#include <cstdint>
#include <set>
#include <tuple>
#include <vector>

namespace summit447 {
using Identity = std::tuple<std::uint32_t,std::uint32_t,std::uint32_t>;
struct NativeRecord {
    Identity identity;
    int itemPosition;
    bool parkedThrow;
    NativeRecord(const Identity& i,int p,bool t=false)
      :identity(i),itemPosition(p),parkedThrow(t){}
};
enum class Result { Match=0, InvalidRecord=1, MissingThrowRecord=2,
                    DuplicateDat=3, DuplicateOnline=4, Differ=5 };
inline bool IsPotion(std::uint32_t code){return (code&0xff000000u)==0x04000000u;}
inline bool ValidIdentity(const Identity& i){return std::get<0>(i)!=0 &&
    std::get<1>(i)!=0 && std::get<2>(i)!=0;}
inline Result Check(const std::vector<NativeRecord>& native,
                    const std::vector<Identity>& online,
                    const std::set<Identity>& throwRegistry){
    std::set<Identity> dat,live,allDat;
    for(const auto& r:native){
        if(r.itemPosition<-1||r.itemPosition>16)return Result::InvalidRecord;
        if(IsPotion(std::get<0>(r.identity)))continue;
        if(!ValidIdentity(r.identity))return Result::InvalidRecord;
        if(!allDat.insert(r.identity).second)return Result::DuplicateDat;
        // ThrowItemInfo is the only native exception to the online ledger.
        // All actual equipped and cursor items MUST be reconciled too.
        if(throwRegistry.find(r.identity)!=throwRegistry.end())continue;
        if(!dat.insert(r.identity).second)return Result::DuplicateDat;
    }
    for(const auto& t:throwRegistry)if(!IsPotion(std::get<0>(t)) &&
       (!ValidIdentity(t)||allDat.find(t)==allDat.end()))
        return Result::MissingThrowRecord;
    for(const auto& id:online){
        if(IsPotion(std::get<0>(id)))continue;
        if(!ValidIdentity(id))return Result::InvalidRecord;
        if(!live.insert(id).second)return Result::DuplicateOnline;
    }
    return dat==live?Result::Match:Result::Differ;
}
} // namespace summit447
