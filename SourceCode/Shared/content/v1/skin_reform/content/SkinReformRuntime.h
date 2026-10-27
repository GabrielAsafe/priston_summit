#pragma once
#include <cstdint>
#include <string>
#include "content/v1/runtime/RuntimeContentPath.h"
#include "content/v1/skin_reform/content/SkinReformPack.h"

namespace summit { namespace content { namespace skin_reform {

inline const SkinReformPack &ActiveSkinReformPack() {
    struct Cache {
        const std::string path;
        const SkinReformPack pack;
        Cache() : path(summit::content::runtime::ResolveRuntimePackPath("skin_reforms.sskm")),
                  pack(path.c_str()) {}
    };
    static const Cache cache;
    return cache.pack;
}
inline bool ResolveSkinReform(std::uint32_t skin, std::uint32_t &target) {
    return ActiveSkinReformPack().Lookup(skin,target);
}

}}}
