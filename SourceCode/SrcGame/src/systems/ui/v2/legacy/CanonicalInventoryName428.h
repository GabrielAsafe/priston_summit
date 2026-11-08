#pragma once
// SUMMIT428A — DISPLAY ONLY: maps native CODE to trusted canonical name.
// Does not alter sITEM, DAT, inventory slots, checksums, or gameplay authority.
#include "systems/ui/v2/pages/CraftUiRegistry426.generated.h"
#include <cstdint>
#include <map>
#include <set>
#include <string>

namespace summit_inventory_name_428 {
struct ItemName { std::string code; std::string name; };
inline const std::map<std::uint32_t, ItemName>& Catalog() {
    static const std::map<std::uint32_t, ItemName> catalog = [] {
        std::map<std::uint32_t, ItemName> rows;
        std::set<std::uint32_t> ambiguous;
        for (std::size_t i=0; i<summit_craft_ui_426::kItemCount; ++i) {
            const auto& source=summit_craft_ui_426::kItems[i];
            if (!source.both || source.native == 0 || !source.code || !source.code[0]) continue;
            const std::string code=source.code;
            const std::string name=source.name ? source.name : "";
            auto found=rows.find(source.native);
            if (found != rows.end()) {
                if (found->second.code != code || found->second.name != name)
                    ambiguous.insert(source.native);
            } else rows.emplace(source.native,ItemName{code,name});
        }
        for (const auto id:ambiguous) rows.erase(id);
        return rows;
    }();
    return catalog;
}
inline std::string Display(std::uint32_t native, const std::string& legacy) {
    const auto& catalog=Catalog();
    const auto found=catalog.find(native);
    if (found==catalog.end()) {
        // Unknown native identity: do not claim that a repeated legacy name is canonical.
        return native ? "[ID " + std::to_string(native) + "]" : "[ID desconhecido]";
    }
    const auto& row=found->second;
    const bool ci=row.code.size()==5 && row.code.compare(0,2,"CI")==0;
    if (!row.name.empty() && row.name!=row.code)
        return "[" + row.code + "] " + row.name;
    if (ci) return "[" + row.code + "]";
    if (!legacy.empty()) return "[" + row.code + "] " + legacy;
    return "[" + row.code + "]";
}
}  // namespace summit_inventory_name_428
