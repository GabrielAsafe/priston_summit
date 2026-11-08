#pragma once
// SUMMIT357: A physical unequip turns an equipment-only DAT row into a bag
// row. Equipped rows may have ItemCount=-1, a stale index or an index also
// held by a bag item. Never reuse that index without checking bag ownership.
// This helper only reserves a FREE legacy sITEM array index; it does not
// mutate inventory, select a bag or grant an equipment permission.
#include <cstddef>
#include <vector>
namespace gameplay { namespace companions { namespace v2 {
inline int FindUnoccupiedUnequipSlot357(const std::vector<int>& bagIndices,
                                       int capacity) {
    if(capacity<=0 || capacity>4096 ||
       bagIndices.size()>static_cast<std::size_t>(capacity)) return -1;
    std::vector<unsigned char> used(static_cast<std::size_t>(capacity),0);
    for(int index:bagIndices) {
        if(index<0 || index>=capacity || used[static_cast<std::size_t>(index)])
            return -1; // corrupt existing bag: fail closed
        used[static_cast<std::size_t>(index)]=1;
    }
    for(int i=0;i<capacity;++i)
        if(!used[static_cast<std::size_t>(i)]) return i;
    return -1;
}
}}}
