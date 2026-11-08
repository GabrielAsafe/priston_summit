#pragma once
// SUMMIT425A: pure, deterministic selection of distinct physical ingredient
// identities for FUTURE durable craft commits. Never changes inventory or DAT.
#include "systems/gameplay/crafting/v2/PhysicalCraftInventoryPlan423.h"
#include <algorithm>
#include <cstdint>
#include <set>
#include <tuple>
#include <vector>

namespace summit_craft_425 {
enum class SelectionStatus { Ready, Missing, Invalid, Ambiguous };
struct Selection {
    SelectionStatus status=SelectionStatus::Invalid;
    std::vector<summit_craft_423::Instance> chosen;
};
inline Selection SelectInstances(const std::vector<summit_craft_423::Ingredient>& required,
                                 const std::vector<summit_craft_423::Instance>& items) {
    Selection out;
    if(required.empty() || required.size()>16u || items.size()>4096u)return out;
    std::set<std::uint32_t> codes;
    for(const auto& in:required)
        if(in.nativeCode==0 || in.required<=0 || in.required>10000 ||
           !codes.insert(in.nativeCode).second) return out;
    std::set<std::uint32_t> slots;
    std::set<std::tuple<std::uint32_t,std::uint32_t,std::uint32_t>> identities;
    for(const auto& item:items) {
        if(!item.nativeCode || !item.key || !item.checksum ||
           !slots.insert(item.slot).second ||
           !identities.insert(std::make_tuple(item.nativeCode,item.key,item.checksum)).second) {
            out.status=SelectionStatus::Ambiguous;return out;
        }
    }
    std::vector<summit_craft_423::Instance> sorted(items);
    std::sort(sorted.begin(),sorted.end(),[](const summit_craft_423::Instance& a,
                                               const summit_craft_423::Instance& b){return a.slot<b.slot;});
    for(const auto& need:required) {
        int found=0;
        for(const auto& item:sorted) {
            if(item.nativeCode!=need.nativeCode || found>=need.required)continue;
            out.chosen.push_back(item);++found;
        }
        if(found!=need.required) {
            out.chosen.clear();out.status=SelectionStatus::Missing;return out;
        }
    }
    out.status=SelectionStatus::Ready;return out;
}
} // namespace summit_craft_425
