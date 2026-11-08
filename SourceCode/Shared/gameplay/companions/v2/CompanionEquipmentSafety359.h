#pragma once
// SUMMIT359: independent, read-only invariants for inventory reconstruction.
// Never infer identity, erase an item or weaken the server's transfer gates.
#include <cstddef>
#include <cstring>
#include <vector>
#include <utility>

namespace gameplay { namespace companions { namespace v2 {

// A trade never moves physically equipped items. Before replacing the LIVE
// arrays, prove that every existing equipped instance remains, at exactly
// the same equipment position and with its complete item payload intact.
// A conservative refusal is preferable to an unacknowledged lost weapon.
template<class Item>
bool EquippedItemsUnchanged359(const Item* beforeA, const Item* beforeB,
    std::size_t capacity, const std::vector<Item>& afterA,
    const std::vector<Item>& afterB) {
    if(!beforeA || !beforeB || !capacity || capacity>4096 ||
       afterA.size()!=capacity || afterB.size()!=capacity) return false;
    const Item* before[2]={beforeA,beforeB};
    const std::vector<Item>* after[2]={&afterA,&afterB};
    // Preserve bag membership too: a reclassification of equipment between
    // bags changes the persistent record segment and is not a regular trade.
    for(unsigned bag=0;bag<2;++bag) {
        std::vector<bool> matched(capacity,false);
        for(std::size_t i=0;i<capacity;++i) {
            const Item& item=before[bag][i];
            if(!item.Flag || item.ItemPosition<=0) continue;
            if(item.ItemPosition>16) return false;
            bool found=false;
            for(std::size_t j=0;j<capacity;++j) {
                const Item& candidate=(*after[bag])[j];
                if(matched[j] || !candidate.Flag ||
                   candidate.ItemPosition!=item.ItemPosition ||
                   candidate.sItemInfo.CODE!=item.sItemInfo.CODE ||
                   candidate.sItemInfo.ItemHeader.Head!=item.sItemInfo.ItemHeader.Head ||
                   candidate.sItemInfo.ItemHeader.dwChkSum!=item.sItemInfo.ItemHeader.dwChkSum ||
                   std::memcmp(&candidate.sItemInfo,&item.sItemInfo,
                               sizeof(item.sItemInfo))!=0) continue;
                matched[j]=true; found=true; break;
            }
            if(!found) return false;
        }
    }
    return true;
}

// Stage a *selected, already-validated* item from secondary bag B into the
// physical equipment segment A. No byte of the record is synthesized; the
// ordinary DAT writer still validates, verifies and atomically commits.
template<class Item>
bool PromoteSecondaryEquipment359(std::vector<Item>& records,
    int& firstSecondary, int originalIndex, int maxBagSlots) {
    if(firstSecondary<0 || originalIndex<firstSecondary ||
       originalIndex>=static_cast<int>(records.size()) ||
       firstSecondary>=maxBagSlots || maxBagSlots<=0 ||
       records[originalIndex].ItemPosition<=0 ||
       records[originalIndex].ItemPosition>16) return false;
    Item moved=records[static_cast<std::size_t>(originalIndex)];
    records.erase(records.begin()+originalIndex);
    records.insert(records.begin()+firstSecondary,std::move(moved));
    ++firstSecondary;
    return true;
}

// No two genuine records may claim the same physical equipment slot.
// Avoid the legacy behavior that silently set Flag=0 on both duplicates.
template<class Item>
bool EquipmentPositionsUnique359(const Item* items,std::size_t count) {
    if(!items || !count || count>4096) return false;
    bool used[17]={};
    for(std::size_t i=0;i<count;++i) {
        const Item& it=items[i];
        if(!it.Flag || it.ItemPosition<=0) continue;
        if(it.ItemPosition>16) return false;
        // 11-13 are potion-stack positions, not generic equipment.
        if(it.ItemPosition>=11 && it.ItemPosition<=13) continue;
        if(used[it.ItemPosition]) return false;
        used[it.ItemPosition]=true;
    }
    return true;
}

}}} // namespace gameplay::companions::v2
