#pragma once
// SUMMIT423A - pure read-only planning. Never grants, removes or saves items.
#include <cstdint>
#include <set>
#include <string>
#include <tuple>
#include <vector>

namespace summit_craft_423 {
struct Ingredient {
    std::string resource;
    std::uint32_t nativeCode;
    int required;
    Ingredient(const std::string& r=std::string(),std::uint32_t n=0,int c=0)
        :resource(r),nativeCode(n),required(c) {}
};
struct Instance {
    std::uint32_t slot, nativeCode, key, checksum;
    Instance(std::uint32_t s=0,std::uint32_t n=0,std::uint32_t k=0,std::uint32_t c=0)
        :slot(s),nativeCode(n),key(k),checksum(c) {}
};
enum class Status { Ready, Missing, InvalidRecipe, AmbiguousInventory };
struct Line {
    std::string resource;
    std::uint32_t nativeCode;
    int required, available;
    Line(const std::string& r=std::string(),std::uint32_t n=0,int c=0,int a=0)
        :resource(r),nativeCode(n),required(c),available(a) {}
};
struct Check {
    Status status;
    std::vector<Line> lines;
    Check():status(Status::InvalidRecipe),lines() {}
};
inline Check Preview(const std::vector<Ingredient>& ingredients,
                     const std::vector<Instance>& instances) {
    Check out;
    if(ingredients.empty() || ingredients.size()>16u || instances.size()>4096u) return out;
    std::set<std::uint32_t> ingredientCodes;
    for(const auto& in:ingredients) {
        if(in.resource.empty() || in.nativeCode==0 || in.required<1 || in.required>10000 ||
           !ingredientCodes.insert(in.nativeCode).second) return out;
        out.lines.push_back(Line{in.resource,in.nativeCode,in.required,0});
    }
    std::set<std::uint32_t> slots;
    std::set<std::tuple<std::uint32_t,std::uint32_t,std::uint32_t>> identities;
    for(const auto& row:instances) {
        if(!row.nativeCode || !row.key || !row.checksum ||
           !slots.insert(row.slot).second ||
           !identities.emplace(row.nativeCode,row.key,row.checksum).second) {
            out.status=Status::AmbiguousInventory;
            out.lines.clear();return out;
        }
        for(auto& line:out.lines)
            if(line.nativeCode==row.nativeCode && line.available<10001) ++line.available;
    }
    out.status=Status::Ready;
    for(const auto& line:out.lines)
        if(line.available<line.required)out.status=Status::Missing;
    return out;
}
} // namespace summit_craft_423
